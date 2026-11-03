"""Test hardware-free estimators, both heading modes, and Python analysis tools."""
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
firmware = root / 'firmware/ImageRastering'
tests = root / 'tests'
header = (firmware / 'Controller.h').read_text()
header = header.replace('#include "Robot.h"', '').replace('#include "RobotWifiAP.h"', '')
header = header.replace('#include "RobotIMU.h"', '')
source = (firmware / 'Controller.cpp').read_text().replace('#include "Controller.h"', '')


def check(name, directory, extra=()):
    binary = directory / name
    command = ['c++', '-std=c++17', '-fsanitize=address,undefined', '-g',
               '-I', str(directory), '-I', str(tests / 'stubs'), '-I', str(firmware),
               str(tests / f'test_{name}.cpp')]
    command += [str(directory / f) for f in extra]
    subprocess.run(command + ['-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)


with tempfile.TemporaryDirectory() as directory:
    temp = Path(directory)
    shutil.copytree(firmware, temp, dirs_exist_ok=True)
    for name, extra in [('transport', ()), ('gyro_heading', ()),
                        ('fused_odometry', ()), ('imu_reader', ('RobotIMU.cpp',))]:
        check(name, temp, extra)
    for fusion in ('true', 'false'):
        variant = temp / f'fusion_{fusion}'
        shutil.copytree(firmware, variant)
        path = variant / 'Config.h'
        config, count = re.subn(r'(\bUSE_HEADING_FUSION\s*=\s*)[^;]+;',
                                rf'\g<1>{fusion};', path.read_text())
        assert count == 1, 'USE_HEADING_FUSION'
        path.write_text(config)
        (variant / 'controller_under_test.inc').write_text(header + '\n' + source)
        check('controller', variant, ('OdometryModel.cpp', 'TaskTimer.cpp'))
subprocess.run([sys.executable, '-m', 'unittest', 'discover', '-s', str(tests), '-p', 'test_*.py', '-v'],
               cwd=root, check=True)
