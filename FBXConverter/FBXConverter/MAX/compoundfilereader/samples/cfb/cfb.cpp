#include <compoundfilereader.h>
#include <utf.h>
#include <string.h>
#include <stdio.h>
#include <memory>
#include <iostream>
#include <iomanip>

using namespace std;

void ShowUsage()
{
    cout <<
        "usage:\n"
        "cfb list FILENAME\n"
        "cfb dump FILENAME STREAM_PATH\n"
        "cfb info FILENAME\n"
        "cfb info FILENAME STREAM_PATH\n"
        << endl;
}

void DumpBuffer(const void* buffer, size_t len)
{
    const unsigned char* str = static_cast<const unsigned char*>(buffer);
    for (size_t i = 0; i < len; i++)
    {
        if (i > 0 && i % 16 == 0)
            cout << endl;
        cout << setw(2) << setfill('0') << hex << static_cast<int>(str[i]) << ' ';
    }
    cout << endl;
}

void DumpText(const char* buffer, size_t len)
{
    cout << std::string(buffer, len) << endl;
}

void OutputFileInfo(const CFB::CompoundFileReader& reader)
{
    const CFB::COMPOUND_FILE_HDR* hdr = reader.GetFileInfo();
    cout
        << "file version: " << hdr->majorVersion << "." << hdr->minorVersion << endl
        << "difat sector: " << hdr->numDIFATSector << endl
        << "directory sector: " << hdr->numDirectorySector << endl
        << "fat sector: " << hdr->numFATSector << endl
        << "mini fat sector: " << hdr->numMiniFATSector << endl;
}

void OutputEntryInfo(const CFB::CompoundFileReader& reader, const CFB::COMPOUND_FILE_ENTRY* entry)
{
    cout
        << "entry type: " << (reader.IsPropertyStream(entry) ? "property" : (reader.IsStream(entry) ? "stream" : "directory")) << endl
        << "color flag: " << entry->colorFlag << endl
        << "creation time: " << entry->creationTime << endl
        << "modified time: " << entry->modifiedTime << endl
        << "child ID: " << entry->childID << endl
        << "left sibling ID: " << entry->leftSiblingID << endl
        << "right sibling ID: " << entry->startSectorLocation << entry->rightSiblingID << endl
        << "start sector: " << entry->startSectorLocation << endl
        << "size: " << entry->size << endl;
}

const void ListDirectory(const CFB::CompoundFileReader& reader)
{
    reader.EnumFiles(reader.GetRootEntry(), -1, 
        [&](const CFB::COMPOUND_FILE_ENTRY* entry, const CFB::utf16string& dir, int level)->void
    {
        bool isDirectory = !reader.IsStream(entry);
        std::string name = UTF16ToUTF8(entry->name);
        std::string indentstr(level * 4 - 4, ' ');
        cout << indentstr.c_str() << (isDirectory ? "[" : "") << name.c_str() << (isDirectory ? "]" : "") << endl;
    });
}

const CFB::COMPOUND_FILE_ENTRY* FindStream(const CFB::CompoundFileReader& reader, const char* streamName)
{
    const CFB::COMPOUND_FILE_ENTRY* ret = nullptr;
    reader.EnumFiles(reader.GetRootEntry(), -1, 
        [&](const CFB::COMPOUND_FILE_ENTRY* entry, const CFB::utf16string& u16dir, int level)->void
    {
        if (reader.IsStream(entry))
        {
            std::string name = UTF16ToUTF8(entry->name);
            if (u16dir.length() > 0)
            {
                std::string dir = UTF16ToUTF8(u16dir.c_str());
                if (strncmp(streamName, dir.c_str(), dir.length()) == 0 &&
                    streamName[dir.length()] == '\\' &&
                    strcmp(streamName + dir.length() + 1, name.c_str()) == 0)
                {
                    ret = entry;
                }
            }
            else
            {
                if (strcmp(streamName, name.c_str()) == 0)
                {
                    ret = entry;
                }
            }
        }
    });
    return ret;
}

int main_internal(int argc, char* argv[])
{
    const char* cmd = nullptr;
    const char* file = nullptr;
    const char* streamName = nullptr;
    bool dumpraw = false;
    for (int i = 1; i < argc; i++)
    {
        if (i == 1)
        {
            cmd = argv[i];
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            dumpraw = true;
        }
        else
        {
            if (file == nullptr)
                file = argv[i];
            else
                streamName = argv[i];
        }
    }

    if (cmd == nullptr || file == nullptr)
    {
        ShowUsage();
        return 1;
    }

    FILE* fp = fopen(file, "rb");
    if (fp == NULL)
    {
        cerr << "read file error" << endl;
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    std::unique_ptr<unsigned char> buffer(new unsigned char[len]);
    fseek(fp, 0, SEEK_SET);
    
    len = fread(buffer.get(), 1, len, fp);
    CFB::CompoundFileReader reader(buffer.get(), len);

    if (strcmp(cmd, "list") == 0)
    {
        ListDirectory(reader);
    }
    else if (strcmp(cmd, "dump") == 0 && streamName != nullptr)
    {
        const CFB::COMPOUND_FILE_ENTRY* entry = FindStream(reader, streamName);
        if (entry == nullptr)
        {
            cerr << "error: stream doesn't exist" << endl;
            return 2;
        }
        cout << "size: " << entry->size << endl;
        if (entry->size > std::numeric_limits<size_t>::max())
        {
            cerr << "error: stream too large" << endl;
            return 2;
        }
        size_t size = static_cast<size_t>(entry->size);
        std::unique_ptr<char> content(new char[size]);
        reader.ReadFile(entry, 0, content.get(), size);
        if (dumpraw)
            DumpText(content.get(), size);
        else
            DumpBuffer(content.get(), size);
    }
    else if (strcmp(cmd, "info") == 0)
    {
        if (streamName == nullptr)
        {
            OutputFileInfo(reader);
        }
        else
        {
            const CFB::COMPOUND_FILE_ENTRY* entry = FindStream(reader, streamName);
            if (entry == NULL)
            {
                cerr << "error: stream doesn't exist" << endl;
                return 2;
            }
            OutputEntryInfo(reader, entry);
        }
    }
    else
    {
        ShowUsage();
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        return main_internal(argc, argv);
    }
    catch (CFB::CFBException& e)
    {
        cerr << "error: " << e.what() << endl;
        return 2;
    }
}

