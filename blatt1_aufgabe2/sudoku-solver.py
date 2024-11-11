import argparse
import pycosat
from time import time

def read_sudoku(filename):
  print(f"Reading Sudoku in file '{filename}'...")
  clues = {}
  with open(filename, 'r', encoding='UTF-8') as file:
    first_line = file.readline().strip()
    d = int(first_line)
    n = d * d
    valid_values = [str(i) for i in range(n+1)]
    row = 0
    for line in file.readlines():
      line = line.strip()
      entries = line.split(" ")
      assert len(entries) == n, f'Malformed line: "{line}"'
      for (column, entry) in enumerate(entries):
        assert entry in valid_values, f'Malformed line: "{line}"'
        if entry != '0':
          clues[(row+1, column+1)] = int(entry)
      row += 1
  assert row == n #, f'Malformed Sudoku ({row} rows given, {n} rows expected)'

  print(f"Found valid Sudoku of size {d}x{d}")
  return d, n, clues

def get_cnf_variable(row, column, value, n):
  assert 1 <= row <= n and 1 <= column <= n and 1 <= value <= n
  return (row - 1) * n * n + (column - 1) * n + value

def encode_as_cnf(clues, n, d):
  print(f"Create CNF for problem...")
  cnf = []

  # Build the clauses in a list
  for row in range(1, n + 1):
    for column in range(1, n + 1):
      # the cell at (row, column) has at least one value
      cnf.append([get_cnf_variable(row, column, value, n) for value in range(1, n + 1)])

      # the cell at (row, column) has at most one value
      for value in range(1, n + 1):
        for walue in range(value + 1, n + 1):
          cnf.append(
            [-get_cnf_variable(row, column, value, n),
            -get_cnf_variable(row, column, walue, n)]
          )

  for value in range(1, n + 1):
    # each row contains the value
    for row in range(1, n + 1):
      cnf.append([get_cnf_variable(row, column, value, n) for column in range(1, n + 1)])
    # each column contains the value
    for column in range(1, n + 1):
      cnf.append([get_cnf_variable(row, column, value, n) for row in range(1, n + 1)])
    # each subgrid contains the value
    for sr in range(0, d):
      for sc in range(0, d):
        cnf.append(
          [get_cnf_variable(sr * d + rd, sc * d + cd, value, n)
          for rd in range(1, d + 1) for cd in range(1, d + 1)]
        )

  # the clues must be respected
  for (row, column) in clues:
    value = clues[(row, column)]
    cnf.append([get_cnf_variable(row, column, value, n)])

  print(f"CNF has {n*n*n} literals and {len(cnf)} clauses.")
  return cnf

def get_position_and_value(cnf_variable, n):
  assert 1 <= cnf_variable <= n * n * n
  row = ((cnf_variable - 1) // (n * n)) + 1
  column = ((cnf_variable - 1) // n % n) + 1
  value = ((cnf_variable - 1) % n) + 1
  assert 1 <= row <= n and 1 <= column <= n and 1 <= value <= n
  assert (row - 1) * n * n + (column - 1) * n + value == cnf_variable
  return (row, column, value)

def solve_and_decode(clues, cnf, n, d, start_time):
  number_cnf_variables = n * n * n

  print(f"Starting picosat...")
  solution = pycosat.solve(cnf)
  print(f"picosat has finished.")
  print(f"--- {time() - start_time:.4f} seconds elapsed ---")

  allocation = {}
  for literal in solution:
    if 0 < literal <= number_cnf_variables:
      (row, column, value) = get_position_and_value(literal, n)
      if (row, column) in allocation:
        print(f'Found a solution with two values for the cell at ({row}, {column})')
      allocation[(row, column)] = value

  # debug
#  validate_solution(allocation, clues, n, d)

  print(f"Solved Sudoku:")
  for row in range(1, n + 1):
    print(' ' + ' '.join([str(allocation[(row, column)]) for column in range(1, n + 1)]))
  print('')

def validate_solution(solution, clues, n, d):
  print(f"Validate solution...")
  # each cell should have a number in it
  for row in range(1, n + 1):
    for column in range(1, n + 1):
      if (row, column) not in solution:
        print(f'A solution with no value for the cell at ({row}, {column})')

  all_values = set(range(1, n + 1))

  # each row should contain all the numbers
  for row in range(1, n + 1):
    values = {solution[(row, column)] for column in range(1, n + 1)}
    if values != all_values:
      print(f'A solution where the row {row} does not contain all the values')

  # each column should contain all the numbers
  for column in range(1, n + 1):
    values = {solution[(row, column)] for row in range(1, n + 1)}
    if values != all_values:
      print(f'A solution where the column {column} does not contain all the values')

  # each subgrid should contain all the numbers
  for sr in range(0, d):
    for sc in range(0, d):
      values = {solution[(sr * d + rd, sc * d + cd)] for rd in range(1, d + 1) for cd in range(1, d + 1)}
    if values != all_values:
      print(f'A solution where a subgrid does not contain all the values')

  # the clues must be respected
  for (row, column) in clues:
    clue = clues[(row, column)]
    if solution[(row, column)] != clue:
      print(f'A solution where the clue at the cell ({row + 1}, {column + 1}) is not respected')
  print(f"Validation was successful.")

argp = argparse.ArgumentParser(
  description='Solve Sudoku problems with a SAT solver.',
  formatter_class=argparse.ArgumentDefaultsHelpFormatter
)
argp.add_argument('clues', type=str, help='the Sudoku instance (a set of clues)')
args = argp.parse_args()

start_time = time()
d, n, clues = read_sudoku(args.clues)
print(f"--- {time() - start_time:.4f} seconds elapsed ---")

cnf = encode_as_cnf(clues, n, d)
print(f"--- {time() - start_time:.4f} seconds elapsed ---")

solve_and_decode(clues, cnf, n, d, start_time)
print(f"--- {time() - start_time:.4f} seconds elapsed ---")
