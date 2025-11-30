import os
import shutil
from dataclasses import dataclass
from enum import IntEnum
import subprocess
import re
import matplotlib.pyplot as plt

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = f"{SCRIPT_DIR}/build"
CMAKE_DIR = f"{SCRIPT_DIR}/.."
EXEC_BUILD_PATH = f"{BUILD_DIR}/src/tajo_2025"
EXEC_PATH = f"{SCRIPT_DIR}/tajo_2025"
OUT_DIR = f"{SCRIPT_DIR}/workdir"
RETRIES = 10
RESULTS_DIR = f"{SCRIPT_DIR}/results"

@dataclass
class TestSpec:
  size_g1: int
  size_g2: int
  density_g1: float
  density_g2: float
  g1_based_on_g2: bool

class AlgoType(IntEnum):
  AStarAccurate = 0
  BruteForceAccurate = 1
  ApproxAStar = 2

def get_algo_name(algo_type: AlgoType) -> str:
  if algo_type == AlgoType.AStarAccurate:
    return "AccurateAStar"
  if algo_type == AlgoType.BruteForceAccurate:
    return "BruteForce"
  return "ApproxAStar"

def compile_project():
  os.makedirs(BUILD_DIR, exist_ok=True)
  os.chdir(BUILD_DIR)

  os.system(f"cmake {CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release")
  os.system("cmake --build . -j 8")

  shutil.copy(EXEC_BUILD_PATH, EXEC_PATH)
  os.chdir(SCRIPT_DIR)

def generate_name(spec: TestSpec, prefix: str = "") -> str:
  return f"{prefix}{spec.size_g1}_{spec.size_g2}_{spec.density_g1}_{spec.density_g2}_{spec.g1_based_on_g2}.txt"

def generate_example(spec: TestSpec, prefix: str = "") -> str:
  name = f"{OUT_DIR}/{generate_name(spec, prefix=prefix)}"
  command = f"./tajo_2025 {name} {name} --gen {spec.size_g1} {spec.size_g2} {spec.density_g1} {spec.density_g2} {str(spec.g1_based_on_g2).lower()}"
  os.system(command)
  return name

def time_algo(file: str, algo_type: AlgoType) -> tuple[float, int]:
  opt = ""
  if algo_type == AlgoType.ApproxAStar:
    opt = "--approx"
  elif algo_type == AlgoType.BruteForceAccurate:
    opt = "--bruteforce"

  command = f'/usr/bin/time -v ./tajo_2025 {file} {file}.out {opt}'

  result = subprocess.run(
    command,
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True
  )

  output = result.stdout
  err_output = result.stderr

  match = re.search(r"Execution Time:\s*([0-9.]+)\s*ms", output)
  if not match:
    raise ValueError("Could not parse execution time from output.")
  exec_time_ms = float(match.group(1))

  match_rss = re.search(r"Maximum resident set size \(kbytes\):\s*(\d+)", err_output)
  if not match_rss:
    raise ValueError("Could not parse RSS from time -v output.")
  rss_kb = int(match_rss.group(1))

  return exec_time_ms, rss_kb


def time_algo_avg(file: str, algo_type: AlgoType, retries: int) -> tuple[float, float]:
  time_sum = 0.0
  rss_sum = 0.0

  for _ in range(retries):
    t, rss = time_algo(file, algo_type)
    time_sum += t
    rss_sum += rss

  return time_sum / retries, rss_sum / retries

def save_plots(test_name: str, algo_type: AlgoType, x_data: list, x_label: str, times: list[float], rss: list[float]):
  algo_name = get_algo_name(algo_type)
  filename = f"{RESULTS_DIR}/{test_name}_{algo_name}.png"

  fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10))

  ax1.plot(x_data, times, marker='o', linestyle='-', color='#1f77b4', linewidth=2)
  ax1.set_title(f'Execution Time: {test_name.replace("_", " ").title()} ({algo_name})', fontsize=12, fontweight='bold')
  ax1.set_xlabel(x_label, fontsize=10)
  ax1.set_ylabel('Time (ms)', fontsize=10)
  ax1.grid(True, which='both', linestyle='--', linewidth=0.5)

  ax2.plot(x_data, rss, marker='s', linestyle='-', color='#ff7f0e', linewidth=2)
  ax2.set_title(f'Memory Usage: {test_name.replace("_", " ").title()} ({algo_name})', fontsize=12, fontweight='bold')
  ax2.set_xlabel(x_label, fontsize=10)
  ax2.set_ylabel('RSS (KB)', fontsize=10)
  ax2.grid(True, which='both', linestyle='--', linewidth=0.5)

  plt.tight_layout(pad=3.0)
  plt.savefig(filename)
  plt.close()

def run_non_accurate_non_based_test(algo_type: AlgoType):
  cases = []
  files = []
  x_values = []
  times = []
  mems = []

  for size_g2 in range(4, 13):
    cases.append(
      TestSpec(
        size_g1=size_g2 - 1,
        size_g2=size_g2,
        density_g1=0.5,
        density_g2=0.8,
        g1_based_on_g2=False,
      )
    )
    x_values.append(size_g2)

  for case in cases:
    files.append(generate_example(case))

  for file in files:
    t, m = time_algo_avg(file, algo_type, RETRIES)
    times.append(t)
    mems.append(m)

  save_plots("non_accurate_non_based_test", algo_type, x_values, "Size G2 (G1 = G2 - 1)", times, mems)

def run_non_accurate_based_test(algo_type: AlgoType):
  cases = []
  files = []
  x_values = []
  times = []
  mems = []

  for size_g2 in range(4, 13):
    cases.append(
      TestSpec(
        size_g1=size_g2 - 1,
        size_g2=size_g2,
        density_g1=0.5,
        density_g2=2.3,
        g1_based_on_g2=True,
      )
    )
    x_values.append(size_g2)

  for case in cases:
    files.append(generate_example(case))

  for file in files:
    t, m = time_algo_avg(file, algo_type, RETRIES)
    times.append(t)
    mems.append(m)

  save_plots("non_accurate_based_test", algo_type, x_values, "Size G2 (G1 = G2 - 1)", times, mems)

def run_non_accurate_non_based_dense_g1_test(algo_type: AlgoType):
  cases = []
  files = []
  x_values = []
  times = []
  mems = []

  for size_g2 in range(4, 13):
    cases.append(
      TestSpec(
        size_g1=size_g2 - 1,
        size_g2=size_g2,
        density_g1=3.5,
        density_g2=0.8,
        g1_based_on_g2=False,
      )
    )
    x_values.append(size_g2)

  for case in cases:
    files.append(generate_example(case))

  for file in files:
    t, m = time_algo_avg(file, algo_type, RETRIES)
    times.append(t)
    mems.append(m)

  save_plots("non_accurate_non_based_dense_g1_test", algo_type, x_values, "Size G2 (G1 = G2 - 1)", times, mems)

def run_non_accurate_non_based_g1_edges_grow(algo_type: AlgoType):
  cases = []
  files = []
  x_values = []
  times = []
  mems = []

  for density_g1_coef in range(1, 7):
    val = 0.4 * density_g1_coef
    cases.append(
      TestSpec(
        size_g1=10,
        size_g2=11,
        density_g1=val,
        density_g2=0.8,
        g1_based_on_g2=False,
      )
    )
    x_values.append(val)

  for case in cases:
    files.append(generate_example(case))

  for file in files:
    t, m = time_algo_avg(file, algo_type, RETRIES)
    times.append(t)
    mems.append(m)

  save_plots("non_accurate_non_based_g1_edges_grow", algo_type, x_values, "Density G1", times, mems)

def run_tests():
  run_non_accurate_non_based_test(AlgoType.BruteForceAccurate)
  run_non_accurate_non_based_test(AlgoType.AStarAccurate)
  run_non_accurate_based_test(AlgoType.BruteForceAccurate)
  run_non_accurate_based_test(AlgoType.AStarAccurate)
  run_non_accurate_non_based_dense_g1_test(AlgoType.BruteForceAccurate)
  run_non_accurate_non_based_dense_g1_test(AlgoType.AStarAccurate)
  run_non_accurate_non_based_g1_edges_grow(AlgoType.BruteForceAccurate)
  run_non_accurate_non_based_g1_edges_grow(AlgoType.AStarAccurate)

def main():
  os.makedirs(OUT_DIR, exist_ok=True)
  os.makedirs(RESULTS_DIR, exist_ok=True)

  compile_project()
  run_tests()


if __name__ == '__main__':
  try:
    main()
  except Exception as e:
    print(f"Failed with exception: {e}")