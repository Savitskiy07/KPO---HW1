import subprocess
import sys
from pathlib import Path
from tempfile import TemporaryDirectory


def run(app, group, commands, expected_code=0):
    result = subprocess.run([app, group], input=commands, capture_output=True,
                            encoding="utf-8", timeout=15)
    assert result.returncode == expected_code, result.stdout + result.stderr
    return result.stdout + result.stderr


def answers(*values):
    return "\n".join(str(value) for value in values) + "\n"


def csv_text(group):
    return (group / "results.csv").read_text(encoding="utf-8-sig")


def check_workflow(app, demo, root):
    group = root / "course"
    screen = run(app, group, demo.read_text(encoding="utf-8"))
    assert "Итог: 68 %" in screen and "Проект [10]" in screen
    assert "Предварительно по оценённым критериям: 0 %" in screen
    assert '1;"Смирнова Анна";2;2;68,00;Готов' in csv_text(group)
    assert '2;"Петров Иван";1;2;;Не выставлен' in csv_text(group)

    edits = (
        answers(7, 1, 1, 12) +          # выше максимума
        answers(7, 1, 1, "не число") +
        answers(7, 1, 1, "5,5") +       # допустимое исправление
        answers(9, 1) +                 # результат после исправления
        answers(8, 1, 2) +              # отмена экзамена
        answers(9, 1, 10, 0)            # пересчёт, экспорт, выход
    )
    screen = run(app, group, edits)
    assert "Балл должен лежать" in screen and "Введите число" in screen
    assert "Итог: 53 %" in screen and "Предварительно по оценённым критериям: 55 %" in screen
    assert '1;"Смирнова Анна";1;2;;Не выставлен' in csv_text(group)

    screen = run(app, group, answers(1, '=Иванов;"А"', 1, "Петров Иван", 2, 4, 6, 10, 5))
    assert "3;\"'=Иванов;\"\"А\"\"\";0;2;;Не выставлен" in csv_text(group)
    assert "4  Петров Иван" in screen and "Экзамен [20]" in screen
    assert "5.5" in run(app, group, "6\n0\n")


def check_damaged_file(app, root):
    group = root / "broken"
    group.mkdir()
    file = group / "journal.pos"
    original = 'POS-JOURNAL 1\n"Группа"\nCRITERIA\n'.encode("utf-8")
    file.write_bytes(original)
    run(app, group, "", expected_code=1)
    assert file.read_bytes() == original


if __name__ == "__main__":
    with TemporaryDirectory() as directory:
        root = Path(directory)
        check_workflow(sys.argv[1], Path(sys.argv[2]), root)
        check_damaged_file(sys.argv[1], root)
    print("OK: console, persistence, CSV and damaged file")
