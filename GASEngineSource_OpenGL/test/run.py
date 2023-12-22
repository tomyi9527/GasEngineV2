import sys
import os
import subprocess
import platform
import shutil
import glob

enable_debug = False

# need env variables:
# 'LANG': 'zh_CN.UTF-8',
# 'LC_ALL': 'zh_CN.UTF-8'

sysstr = platform.system()
if enable_debug:
    path_to_output = R''
    if(sysstr == "Windows"):
        path_to_GasEngineV2 = R'D:\render_test\GASEngineV2'
        path_to_run = R'D:\OSR_test'
        path_to_executable = R'D:\OSR_test\App.ArthubScreenShot.exe'
    else:
        path_to_GasEngineV2 = R'/data/home/beanpliu/GASEngineV2'
        path_to_run = R'/data/home/beanpliu/GasEngine_OSR/build/app'
        path_to_executable = R'/data/home/beanpliu/GasEngine_OSR/build/app/App.ArthubScreenShot'
else:
    if len(sys.argv) >= 5:
        path_to_run = sys.argv[1]
        path_to_executable = sys.argv[2]
        path_to_GasEngineV2 = sys.argv[3]
        path_to_output = sys.argv[4]
    else:
        print('Instruction:')
        print('python run.py PATH_TO_CWD PATH_TO_EXECUTABLE PATH_TO_GASENGINEV2_ROOT PATH_TO_OUTPUT')
        exit(-1)

if(sysstr == "Windows"):
    path_to_fbx_converter = os.path.join(path_to_GasEngineV2, 'build/FBXConverter_release.exe')
else:
    path_to_fbx_converter = os.path.join(path_to_GasEngineV2, 'build/FBXConverter_release')
parameter_table = [
    ["assets/airship/airship.fbx", "airship/"],
    ["assets/bristleback/bristleback.FBX", "bristleback/"],
    ["assets/dog/dog.fbx", "dog/"],
    ["assets/fbx/Bamboo_Stage3_B/Bamboo_Stage3_B.FBX",
        "Bamboo_Stage3_B/"],
    ["assets/fbx/Bamboo_stage3_C/Bamboo_stage3_C.FBX",
        "Bamboo_stage3_C/"],
    ["assets/fbx/bush_Stage1/bush_Stage1.FBX", "bush_Stage1/"],
    ["assets/fbx/bushDwarfPine_Stage2/bushDwarfPine_Stage2.FBX",
        "bushDwarfPine_Stage2/"],
    ["assets/fbx/cactusOpuntia_Stage2/cactusOpuntia_Stage2.FBX",
        "cactusOpuntia_Stage2/"],
    ["assets/fbx/Carrot/Carrot.FBX", "Carrot/"],
    ["assets/fbx/OilStorage/OilStorage.fbx", "OilStorage/"],
    ["assets/fbx/Orchid_Stage2/Orchid_Stage2.FBX", "Orchid_Stage2/"],
    ["assets/fbx/Tank/Tank.fbx", "Tank/"],
    ["assets/fbx/vg_mush_Glowing_Mushroom/vg_mush_Glowing_Mushroom.FBX",
        "vg_mush_Glowing_Mushroom/"],
    ["assets/gandalf/gandalf.fbx", "gandalf/"],
    ["assets/girlwalk/girlwalk.fbx", "girlwalk/"],
    ["assets/jacky/jacky.fbx", "jacky/"],
    ["assets/jinglingwangzi/jinglingwangzi.FBX", 'jinglingwangzi/'],
    ["assets/mocap/qlyy/qlyy.fbx", "qlyy/"],
    ["assets/mocap/xue/xue.fbx", "xue/"],
    ["assets/sasuke/sasuke.fbx", "sasuke/"],
    ["assets/smurf/smurf.fbx", "smurf/"],
    ["assets/test1/SK_Buggy_Vehicle.FBX", "test1/"],
    ["assets/test2/Aircraft_C130_Set.FBX", "test2/"],
    ["assets/test3/Run_Fwd.FBX", "test3/"],
    ["assets/test4/HeroTPP.FBX", "test4/"],
    ["assets/test5/ACD_Ability_Bow_FreeAim_版本1.fbx", "test5/"],
]

os.chdir(path_to_run)

my_env = {**os.environ,
          'LD_LIBRARY_PATH': '/usr/lib:/usr/local/lib',
          'LIBGL_ALWAYS_SOFTWARE': 'true',
          'GALLIUM_DRIVER': 'llvmpipe',
          'MESA_GL_VERSION_OVERRIDE': '4.6',
          'MESA_GLSL_VERSION_OVERRIDE': '460',
          'LANG': 'zh_CN.UTF-8',
          'LC_ALL': 'zh_CN.UTF-8'
          }

cmd_record = []

for item in parameter_table:
    output_dir = os.path.join(path_to_output, item[1])
    tmp_dir = os.path.join(output_dir, 'converted')
    if not os.path.isdir(output_dir):
        os.makedirs(output_dir)
    target_fbx = os.path.join(path_to_GasEngineV2, item[0])
    cvt_command = [
        path_to_fbx_converter,
        '-f', target_fbx,
        '-u', tmp_dir + '/',
        '-tgas2',
        '-o0xffffffff',
        '-b'
    ]
    print('running: {}'.format(' '.join(cvt_command)))
    p = subprocess.Popen(cvt_command , env=my_env, cwd=path_to_run, stdout=subprocess.DEVNULL, encoding='utf-8')
    p.wait()
    print('returns: {}'.format(p.returncode))
    for file in glob.glob(os.path.join(os.path.dirname(target_fbx), "*")):
        if os.path.isfile(file) and not file.lower().endswith('.fbx'):
            shutil.copyfile(file, os.path.join(tmp_dir, os.path.basename(file)))

    fbx_name = os.path.split(item[0])[1]
    my_command = [
        path_to_executable,
        '-input', os.path.join(tmp_dir, fbx_name),
        '-output', output_dir,
        '-window_height=480', '-window_width=480', '-save_gif=true'
    ]
    print('running: {}'.format(my_command))
    p = subprocess.Popen(my_command, env=my_env, cwd=path_to_run, stdout=subprocess.DEVNULL, encoding='utf-8')
    p.wait()
    print('returns: {}'.format(p.returncode))
    cmd_record.append([my_command, p.returncode])
    if len(tmp_dir) > 0:
        shutil.rmtree(tmp_dir)

# print('')
# for record in cmd_record:
#     print('command {} returns {}'.format(' '.join(record[0]), record[1]))

print('')
error_count = 0
for record in cmd_record:
    if record[1] != 0:
        error_count = error_count + 1
        print('{} --> {} returns {}'.format(record[0][2], record[0][4], record[1]))

exit(error_count)