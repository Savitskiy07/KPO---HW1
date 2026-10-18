#!/usr/bin/env python3

import argparse
import gzip
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile


SOURCE_DIRS = ("domain", "services", "composition", "console")


def run(command, directory):
    result = subprocess.run(
        command, cwd=directory, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace"
    )
    if result.returncode:
        raise RuntimeError(
            "Command failed: " + repr(command) + "\n" + result.stdout
        )
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default="g++", help="GCC C++ compiler")
    parser.add_argument("--gcov", default="gcov", help="gcov matching the compiler")
    parser.add_argument("--output", default="build-coverage",
                        help="Report parent directory, relative to project root")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    compiler_version = run([args.cxx, "--version"], root).splitlines()[0]
    gcov_version = run([args.gcov, "--version"], root).splitlines()[0]
    print(compiler_version, flush=True)
    print(gcov_version, flush=True)

    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)
    build = Path(tempfile.mkdtemp(prefix="run-", dir=output))
    print("Fresh coverage directory:", build, flush=True)

    sources = sorted(
        path for name in SOURCE_DIRS
        for path in (root / "src" / name).rglob("*.cpp")
    )
    tests = sorted((root / "tests").rglob("*.cpp"))
    if not sources or not tests:
        raise RuntimeError("Project source files or tests were not found")

    flags = [
        "-std=c++17", "-O0", "-g", "--coverage",
        "-Wall", "-Wextra", "-Wpedantic",
        "-finput-charset=UTF-8", "-fexec-charset=UTF-8",
        "-I", str(root / "src"),
        "-isystem", str(root / "third_party" / "doctest")
    ]
    objects = []
    compiler_log = []
    for source in sources + tests:
        stem = "_".join(source.relative_to(root).with_suffix("").parts)
        obj = build / (stem + ".o")
        compiler_log.append(run(
            [args.cxx, *flags, "-c", str(source), "-o", str(obj)], build
        ))
        notes = obj.with_suffix(".gcno")
        if not notes.is_file() or notes.stat().st_size == 0:
            raise RuntimeError("Missing or empty GCC coverage notes: " + str(notes))
        objects.append(obj)

    executable = build / ("HW1_tests.exe" if os.name == "nt" else "HW1_tests")
    compiler_log.append(run(
        [args.cxx, "--coverage", *map(str, objects), "-o", str(executable)], build
    ))
    (build / "compiler.log").write_text("".join(compiler_log), encoding="utf-8")
    test_output = run([str(executable)], build)
    (build / "tests.log").write_text(test_output, encoding="utf-8")
    print(test_output, end="", flush=True)

    gcov_log = []
    for obj in objects:
        gcov_log.append(run(
            [args.gcov, "--json-format", str(obj.with_suffix(".gcno"))], build
        ))
    (build / "gcov.log").write_text("".join(gcov_log), encoding="utf-8")

    reports = sorted(build.glob("*.gcov.json.gz"))
    if len(reports) != len(objects):
        raise RuntimeError("Expected one gcov JSON report per translation unit")

    lines = {}
    for report in reports:
        with gzip.open(report, "rt", encoding="utf-8") as stream:
            data = json.load(stream)
        directory = Path(data.get("current_working_directory", str(build)))
        for source in data["files"]:
            path = Path(source["file"])
            if not path.is_absolute():
                path = directory / path
            try:
                relative = path.resolve().relative_to(root)
            except ValueError:
                continue
            if (len(relative.parts) < 3 or relative.parts[0] != "src"
                    or relative.parts[1] not in SOURCE_DIRS
                    or relative.suffix not in (".cpp", ".h")):
                continue
            hits = lines.setdefault(relative.as_posix(), {})
            for line in source["lines"]:
                number = line["line_number"]
                hits[number] = hits.get(number, False) or line["count"] > 0

    rows = []
    for name, hits in sorted(lines.items()):
        if hits:
            rows.append({
                "file": name, "covered": sum(hits.values()), "total": len(hits),
                "uncovered_lines": sorted(n for n, hit in hits.items() if not hit)
            })
    covered = sum(row["covered"] for row in rows)
    total = sum(row["total"] for row in rows)
    if not total:
        raise RuntimeError("No executable lines found within the coverage scope")

    summary = {
        "metric": "gcov executable line coverage",
        "compiler": compiler_version, "gcov": gcov_version,
        "included_directories": ["src/" + name for name in SOURCE_DIRS],
        "excluded": ["src/main.cpp", "Windows startup code", "tests",
                     "third_party", "standard library", "tools"],
        "covered": covered, "total": total,
        "percent": 100.0 * covered / total, "files": rows
    }
    report_path = build / "coverage-summary.json"
    report_path.write_text(
        json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    for row in rows:
        print("{covered:4}/{total:<4} {file}".format(**row))
    print("TOTAL: {}/{} executable lines ({:.2f}%)".format(
        covered, total, summary["percent"]
    ))
    print("Summary:", report_path)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, ValueError) as error:
        print("Coverage failed:", error, file=sys.stderr)
        sys.exit(1)
