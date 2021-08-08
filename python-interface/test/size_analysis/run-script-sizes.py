# ==============================================================================
#  This file is part of the SPNC project under the Apache License v2.0 by the
#  Embedded Systems and Applications Group, TU Darmstadt.
#  For the full copyright and license information, please view the LICENSE
#  file that was distributed with this source code.
#  SPDX-License-Identifier: Apache-2.0
# ==============================================================================

import csv
import fire
import os
import re
import subprocess


def parse_output(output):
    """
    Example data found in output:

    [...]

    #ops: 2778

    [...]

    STATUS OK
    """
    size_re = re.compile(r"#ops:\s+(\d+)")
    status_re = re.compile(r"STATUS OK")
    size = None
    status = False
    for line in output.splitlines():
        if m := size_re.match(line):
            size = int(m.group(1))
        elif status_re.match(line):
            status = True
    if not status or size is None:
        raise RuntimeError("Size computation failed")
    return {"#lospn ops": size}


def invokeCompileAndExecute(model):
    command = ["python3", os.path.join(os.path.dirname(os.path.realpath(__file__)), "cpu-execution-sizes.py")]
    command.extend((model[0], model[1]))
    command.extend((str(True), str(None), str(False)))
    run_result = subprocess.run(command, capture_output=True, text=True)
    if run_result.returncode == 0:
        return parse_output(run_result.stdout)
    else:
        print(f"Compilation and execution of {model[0]} failed")
        print(run_result.stdout)
        print(run_result.stderr)
    return None


def get_ratspns(ratspnDir: str):
    models = []
    for subdir, dirs, files in os.walk(ratspnDir):
        for file in files:
            baseName = os.path.basename(file)
            extension = os.path.splitext(baseName)[-1].lower()
            modelName = subdir.replace(ratspnDir, '') + "_" + os.path.splitext(baseName)[0]
            if extension == ".bin":
                modelFile = os.path.join(subdir, file)
                models.append((modelName, modelFile))
    print(f"Number of RAT-SPN models found: {len(models)}")
    return models


def get_speakers(speakersDir: str):
    models = []
    for subdir, dirs, files in os.walk(speakersDir):
        for file in files:
            baseName = os.path.basename(file)
            extension = os.path.splitext(baseName)[-1].lower()
            modelName = os.path.splitext(baseName)[0]
            if extension == ".bin":
                modelFile = os.path.join(subdir, file)
                models.append((modelName, modelFile))
    print(f"Number of speaker models found: {len(models)}")
    return models


def traverseModels(speakersDir: str, ratspnDir: str, logDir: str):
    models = []
    models.extend(get_ratspns(ratspnDir))
    models.extend(get_speakers(speakersDir))

    # Sort models s.t. smaller ones are executed first
    counter = 0
    models = sorted(models, key=lambda m: os.path.getsize(m[1]))
    for m in [models[-13], models[-12], models[-11], models[-10], models[-9], models[-8], models[-7], models[-6],
              models[-5], models[-4], models[-3], models[-2], models[-1]]:
        print(f"Skipping model {m} because of traversal limit in words problems.")
        print(f"\tFile size of model: {os.path.getsize(m[1])} bytes")
        counter = counter + 1

    for m in models[:-13]:
        counter = counter + 1
        print(f"Current model ({counter}/{len(models)}): {m[0]}")
        continue
        sizes = invokeCompileAndExecute(m)
        data = {"Name": m[0], "file size (bytes)": os.path.getsize(m[1])}
        data.update(sizes)

        log_file_all = os.path.join(logDir, "lo_spn_sizes.csv")
        file_exists = os.path.isfile(log_file_all)
        if not os.path.isdir(logDir):
            os.mkdir(logDir)
        with open(log_file_all, 'a') as log_file:
            writer = csv.DictWriter(log_file, delimiter=",", fieldnames=data.keys())
            if not file_exists:
                writer.writeheader()
            writer.writerow(data)


if __name__ == '__main__':
    fire.Fire(traverseModels)
