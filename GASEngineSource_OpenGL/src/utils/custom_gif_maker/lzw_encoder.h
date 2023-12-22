#pragma once
/*
 LZWEncoder.js
 Authors
 Kevin Weiner (original Java version - kweiner@fmsware.com)
 Thibault Imbert (AS3 version - bytearray.org)
 Johan Nordberg (JS version - code@johan-nordberg.com)
 Acknowledgements
 GIFCOMPR.C - GIF Image compression routines
 Lempel-Ziv compression based on 'compress'. GIF modifications by
 David Rowley (mgardi@watdcsu.waterloo.edu)
 GIF Image compression - modified 'compress'
 Based on: compress.c - File compression ala IEEE Computer, June 1984.
 By Authors: Spencer W. Thomas (decvax!harpo!utah-cs!utah-gr!thomas)
 Jim McKie (decvax!mcvax!jim)
 Steve Davies (decvax!vax135!petsd!peora!srd)
 Ken Turkowski (decvax!decwrl!turtlevax!ken)
 James A. Woods (decvax!ihnp4!ames!jaw)
 Joe Orost (decvax!vax135!petsd!joe)
*/
#include <cmath>
#include <ostream>
#include <vector>
#include <sstream>
#include <algorithm>

constexpr static int BITS = 12;
constexpr static int HSIZE = 5003; // 80% occupancy
constexpr static uint16_t masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F,
0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

class LZWEncoder {
public:
   LZWEncoder(int32_t width_in, int32_t height_in, const std::vector<uint8_t>& pixels_in, int32_t colorDepth) {
       width = width_in;
       height = height_in;
       initCodeSize = std::max(2, colorDepth);
       pixels = pixels_in.data();
   }

   void encode(std::ostream& outs) {
       outs.write(reinterpret_cast<const char*>(&initCodeSize), 1); // write "initial code size" byte
       remaining = width * height; // reset navigation variables
       curPixel = 0;
       compress(initCodeSize + 1, outs); // compress and write the pixel data
       outs.write(reinterpret_cast<const char*>(&terminator), 1);
   }

protected:
   // Add a character to the end of the current packet, and if it is 254
   // characters, flush the packet to disk.
   void char_out(uint8_t c, std::ostream& outs) {
       accum[a_count++] = c;
       if (a_count >= 254) flush_char(outs);
   }

   // Clear out the hash table
   // table clear for block compress
   void cl_block(std::ostream& outs) {
       cl_hash(HSIZE);
       free_ent = ClearCode + 2;
       clear_flg = true;
       output(ClearCode, outs);
   }

   // Reset code table
   void cl_hash(int hsize) {
       for (int i = 0; i < hsize; ++i) htab[i] = -1;
   }

   void compress(uint8_t init_bits, std::ostream& outs) {
       int32_t fcode, i, disp, hsize_reg, hshift;
       // Set up the globals: g_init_bits - initial number of bits
       n_bits = g_init_bits = init_bits;
       clear_flg = false;
       maxcode = MAXCODE(n_bits); // init_bits 能表示的最大值

       ClearCode = 1 << (init_bits - 1); // 0x1000...000
       EOFCode = ClearCode + 1;  // 0x1000...001
       free_ent = ClearCode + 2; // 0x1000...010

       a_count = 0; // clear packet

       int32_t ent, c;
       ent = nextPixel(); // first value

       hshift = 0;
       for (fcode = HSIZE; fcode < 65536; fcode *= 2) ++hshift;
       hshift = 8 - hshift; // set hash code range bound
       hsize_reg = HSIZE;
       cl_hash(hsize_reg); // clear hash table

       output(ClearCode, outs);

       bool continue_flag = false;
       while (remaining > 0) {
           c = nextPixel();
           fcode = (c << BITS) + ent;
           i = (c << hshift) ^ ent; // xor hashing
           if (htab[i] == fcode) {
               ent = codetab[i];
               continue;
           } else if (htab[i] >= 0) { // non-empty slot
               disp = hsize_reg - i; // secondary hash (after G. Knott)
               if (i == 0) disp = 1;
               do {
                   if ((i -= disp) < 0) i += hsize_reg;
                   if (htab[i] == fcode) {
                       ent = codetab[i];
                       continue_flag = true;
                       break;
                   }
               } while (htab[i] >= 0);
               if (continue_flag) {
                   continue_flag = false;
                   continue;
               }
           }
           output(ent, outs);
           ent = c;
           if (free_ent < 1 << BITS) {
               codetab[i] = free_ent++; // code -> hashtable
               htab[i] = fcode;
           } else {
               cl_block(outs);
           }
       }

       // Put out the final code.
       output(ent, outs);
       output(EOFCode, outs);
   }

   // Flush the packet to disk, and reset the accumulator
   void flush_char(std::ostream& outs) {
       if (a_count > 0) {
           outs.write(reinterpret_cast<const char*>(&a_count), 1);
           outs.write(reinterpret_cast<const char*>(accum.data()), a_count);
           a_count = 0;
       }
   }

   int32_t MAXCODE(uint8_t n_bits) {
       return (1 << n_bits) - 1;
   }

   // Return the next pixel from the image
   int32_t nextPixel() {
       if (remaining == 0) return -1;
       --remaining;
       return pixels[curPixel++];
   }

   void output(int32_t code, std::ostream& outs) {
       cur_accum &= masks[cur_bits];

       if (cur_bits > 0) cur_accum |= (code << cur_bits);
       else cur_accum = code;

       cur_bits += n_bits;

       while (cur_bits >= 8) {
           char_out((cur_accum & 0xff), outs);
           cur_accum >>= 8;
           cur_bits -= 8;
       }

       // If the next entry is going to be too big for the code size,
       // then increase it, if possible.
       if (free_ent > maxcode || clear_flg) {
           if (clear_flg) {
               maxcode = MAXCODE(n_bits = g_init_bits);
               clear_flg = false;
           } else {
               ++n_bits;
               if (n_bits == BITS) maxcode = 1 << BITS;
               else maxcode = MAXCODE(n_bits);
           }
       }

       if (code == EOFCode) {
           // At EOF, write the rest of the buffer.
           while (cur_bits > 0) {
               char_out((cur_accum & 0xff), outs);
               cur_accum >>= 8;
               cur_bits -= 8;
           }
           flush_char(outs);
       }
   }

protected:
   uint8_t initCodeSize = 2;
   constexpr static uint8_t terminator = 0;

   std::vector<uint8_t> accum = std::vector<uint8_t>(256, 0);
   std::vector<int32_t> htab = std::vector<int32_t>(HSIZE, 0);
   std::vector<int32_t> codetab = std::vector<int32_t>(HSIZE, 0);

   int32_t cur_accum = 0, cur_bits = 0;
   uint8_t a_count = 0;
   int32_t free_ent = 0; // first unused entry
   int32_t maxcode = 0;

   // block compression parameters -- after all codes are used up,
   // and compression rate changes, start over.
   bool clear_flg = false;

   // Algorithm: use open addressing double hashing (no chaining) on the
   // prefix code / next character combination. We do a variant of Knuth's
   // algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
   // secondary probe. Here, the modular division first probe is gives way
   // to a faster exclusive-or manipulation. Also do block compression with
   // an adaptive reset, whereby the code table is cleared when the compression
   // ratio decreases, but after the table fills. The variable-length output
   // codes are re-sized at this point, and a special CLEAR code is generated
   // for the decompressor. Late addition: construct the table according to
   // file size for noticeable speed improvement on small files. Please direct
   // questions about this implementation to ames!jaw.
   int32_t g_init_bits = 0, n_bits = 0, ClearCode = 0, EOFCode = 0;
   int32_t width = 0, height = 0;
   int32_t remaining = 0, curPixel = 0;
   const uint8_t* pixels = nullptr;
};