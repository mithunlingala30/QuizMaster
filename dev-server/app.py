"""
QuizMaster - FastAPI dev/testing backend
=========================================
THIS IS NOT THE REAL SUBMISSION. The actual project backend is the C/CGI
code in ../backend/*.c, which is what demonstrates file handling for the
C Programming subject and what you present/submit.

This file exists purely so you can run and click through the whole
frontend on native Windows (or Mac/Linux) without installing WSL, gcc,
or an Apache/CGI setup - useful while you're designing pages or want to
demo the UI quickly. It exposes the exact same routes, the exact same
request field names, and the exact same JSON response shapes as the 10
C programs, and it reads/writes the exact same pipe-delimited files in
../data/ - so data created here is 100% compatible with the C backend
and vice versa (e.g. register a student here, then later log in against
the real C binaries with the same data/users.txt, or the other way
around).

Run:
    cd dev-server
    pip install -r requirements.txt
    uvicorn app:app --reload --port 8000

Then open http://localhost:8000/index.html
"""

import os
from datetime import datetime
from typing import Optional

from fastapi import FastAPI, Form, Query
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, "..", "data")
FRONTEND_DIR = os.path.join(BASE_DIR, "..", "frontend")

USERS_FILE = os.path.join(DATA_DIR, "users.txt")
ADMIN_FILE = os.path.join(DATA_DIR, "admin.txt")
QUESTIONS_FILE = os.path.join(DATA_DIR, "questions.txt")
SCORES_FILE = os.path.join(DATA_DIR, "scores.txt")

app = FastAPI(title="QuizMaster dev backend (mirrors the C/CGI programs)")


# --------------------------------------------------------------------
# Small helpers that mirror common.h's fgets()+split_line() pattern
# --------------------------------------------------------------------
def read_records(path, field_count):
    """Reads a pipe-delimited file, returns list of field-lists.
    Skips blank lines and malformed rows, same as split_line() in the
    C version returning NULL/skip on a field-count mismatch."""
    if not os.path.exists(path):
        return []
    records = []
    with open(path, "r", encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n").rstrip("\r")
            if not line:
                continue
            fields = line.split("|")
            if len(fields) == field_count:
                records.append(fields)
    return records


def append_record(path, fields):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "a", encoding="utf-8") as fh:
        fh.write("|".join(fields) + "\n")


def rewrite_records(path, records):
    """Equivalent of the C read->temp-file->rename update/delete pattern."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as fh:
        for fields in records:
            fh.write("|".join(fields) + "\n")


def today_str():
    return datetime.now().strftime("%d %b %Y")


# --------------------------------------------------------------------
# student_register  (POST)  - mirrors backend/student_register.c
# --------------------------------------------------------------------
@app.post("/cgi-bin/student_register")
def student_register(
    studentId: str = Form(""),
    name: str = Form(""),
    email: str = Form(""),
    username: str = Form(""),
    password: str = Form(""),
    confirmPassword: str = Form(""),
):
    if not (studentId and name and email and username and password):
        return {"success": False, "message": "All fields are required."}
    if "@" not in email or "." not in email:
        return {"success": False, "message": "Please enter a valid email address."}
    if password != confirmPassword:
        return {"success": False, "message": "Passwords do not match."}

    for f in read_records(USERS_FILE, 5):
        if f[0] == studentId:
            return {"success": False, "message": "Student ID already registered."}
        if f[3] == username:
            return {"success": False, "message": "Username already taken."}

    append_record(USERS_FILE, [studentId, name, email, username, password])
    return {"success": True, "message": "Registration successful. You can now log in."}


# --------------------------------------------------------------------
# student_login  (POST)  - mirrors backend/student_login.c
# --------------------------------------------------------------------
@app.post("/cgi-bin/student_login")
def student_login(username: str = Form(""), password: str = Form("")):
    if not (username and password):
        return {"success": False, "message": "Username and password are required."}

    for f in read_records(USERS_FILE, 5):
        studentId, name, email, uname, pwd = f
        if uname == username:
            if pwd == password:
                return {"success": True, "studentId": studentId, "name": name, "username": uname}
            return {"success": False, "message": "Incorrect password."}

    return {"success": False, "message": "No account found with that username."}


# --------------------------------------------------------------------
# admin_login  (POST)  - mirrors backend/admin_login.c
# --------------------------------------------------------------------
@app.post("/cgi-bin/admin_login")
def admin_login(username: str = Form(""), password: str = Form("")):
    if not (username and password):
        return {"success": False, "message": "Username and password are required."}

    for f in read_records(ADMIN_FILE, 2):
        if f[0] == username and f[1] == password:
            return {"success": True, "message": f"Welcome, {username}."}

    return {"success": False, "message": "Invalid admin credentials."}


# --------------------------------------------------------------------
# get_questions  (GET)  - mirrors backend/get_questions.c
#   ?admin=1 -> includes correctAnswer/marks (manage/edit-question pages)
#   no param -> options only (quiz-taking; answer key withheld)
# --------------------------------------------------------------------
@app.get("/cgi-bin/get_questions")
def get_questions(admin: Optional[str] = Query(None)):
    is_admin = admin == "1"
    out = []
    for f in read_records(QUESTIONS_FILE, 10):
        qid, question, a, b, c, d, correct, category, difficulty, marks = f
        item = {
            "id": int(qid), "question": question,
            "optionA": a, "optionB": b, "optionC": c, "optionD": d,
            "category": category, "difficulty": difficulty,
        }
        if is_admin:
            item["correctAnswer"] = correct
            item["marks"] = int(marks)
        out.append(item)
    return out


# --------------------------------------------------------------------
# add_question  (POST)  - mirrors backend/add_question.c
# --------------------------------------------------------------------
@app.post("/cgi-bin/add_question")
def add_question(
    question: str = Form(""), optionA: str = Form(""), optionB: str = Form(""),
    optionC: str = Form(""), optionD: str = Form(""), correctAnswer: str = Form(""),
    category: str = Form(""), difficulty: str = Form(""), marks: str = Form(""),
):
    if not (question and optionA and optionB and optionC and optionD and correctAnswer):
        return {"success": False, "message": "All question fields are required."}

    existing = read_records(QUESTIONS_FILE, 10)
    next_id = max([int(f[0]) for f in existing], default=0) + 1

    try:
        marks_val = int(marks)
        if marks_val <= 0:
            marks_val = 1
    except ValueError:
        marks_val = 1

    append_record(QUESTIONS_FILE, [
        str(next_id), question, optionA, optionB, optionC, optionD,
        correctAnswer.upper()[:1], category, difficulty, str(marks_val),
    ])
    return {"success": True, "message": "Question added.", "id": next_id}


# --------------------------------------------------------------------
# edit_question  (POST)  - mirrors backend/edit_question.c
# --------------------------------------------------------------------
@app.post("/cgi-bin/edit_question")
def edit_question(
    id: str = Form(""), question: str = Form(""), optionA: str = Form(""),
    optionB: str = Form(""), optionC: str = Form(""), optionD: str = Form(""),
    correctAnswer: str = Form(""), category: str = Form(""),
    difficulty: str = Form(""), marks: str = Form(""),
):
    target_id = int(id) if id.isdigit() else -1
    try:
        marks_val = int(marks)
        if marks_val <= 0:
            marks_val = 1
    except ValueError:
        marks_val = 1

    records = read_records(QUESTIONS_FILE, 10)
    found = False
    new_records = []
    for f in records:
        if int(f[0]) == target_id:
            new_records.append([
                str(target_id), question, optionA, optionB, optionC, optionD,
                correctAnswer.upper()[:1], category, difficulty, str(marks_val),
            ])
            found = True
        else:
            new_records.append(f)

    if found:
        rewrite_records(QUESTIONS_FILE, new_records)
        return {"success": True, "message": "Question updated."}
    return {"success": False, "message": "Question ID not found."}


# --------------------------------------------------------------------
# delete_question  (GET, matching how the frontend calls it via apiGet)
#   - mirrors backend/delete_question.c
# --------------------------------------------------------------------
@app.get("/cgi-bin/delete_question")
def delete_question(id: str = Query("")):
    target_id = int(id) if id.isdigit() else -1
    records = read_records(QUESTIONS_FILE, 10)
    found = any(int(f[0]) == target_id for f in records)
    if found:
        rewrite_records(QUESTIONS_FILE, [f for f in records if int(f[0]) != target_id])
        return {"success": True, "message": "Question deleted."}
    return {"success": False, "message": "Question ID not found."}


# --------------------------------------------------------------------
# submit_quiz  (POST)  - mirrors backend/submit_quiz.c
#   answers format sent by quiz.html: "id:letter,id:letter,..."
#   Scoring happens here on the server, never trusting the browser -
#   same principle as the C version.
# --------------------------------------------------------------------
@app.post("/cgi-bin/submit_quiz")
def submit_quiz(username: str = Form(""), quizName: str = Form(""), answers: str = Form("")):
    bank = {}
    for f in read_records(QUESTIONS_FILE, 10):
        qid, question, a, b, c, d, correct, category, difficulty, marks = f
        bank[int(qid)] = {"question": question, "correct": correct}

    total = correct_count = unanswered = 0
    details = []
    for pair in answers.split(","):
        if ":" not in pair:
            continue
        qid_str, given = pair.split(":", 1)
        if not qid_str.strip().isdigit():
            continue
        qid = int(qid_str)
        if qid not in bank:
            continue
        total += 1
        is_correct = bool(given) and given == bank[qid]["correct"]
        if not given:
            unanswered += 1
        if is_correct:
            correct_count += 1
        details.append({
            "question": bank[qid]["question"],
            "yourAnswer": given,
            "correctAnswer": bank[qid]["correct"],
            "isCorrect": is_correct,
        })

    percentage = round(100.0 * correct_count / total, 2) if total > 0 else 0.0
    date_str = today_str()

    append_record(SCORES_FILE, [username, quizName, date_str, str(total), str(correct_count), f"{percentage:.2f}"])

    return {
        "success": True, "total": total, "correct": correct_count,
        "wrong": total - correct_count - unanswered, "unanswered": unanswered,
        "percentage": percentage, "date": date_str, "details": details,
    }


# --------------------------------------------------------------------
# get_results  (GET)  - mirrors backend/get_results.c
# --------------------------------------------------------------------
@app.get("/cgi-bin/get_results")
def get_results(username: Optional[str] = Query(None)):
    out = []
    for f in read_records(SCORES_FILE, 6):
        uname, quizName, date, total, correct, percentage = f
        if username and uname != username:
            continue
        out.append({
            "username": uname, "quizName": quizName, "date": date,
            "total": int(total), "correct": int(correct), "percentage": float(percentage),
        })
    return out


# --------------------------------------------------------------------
# get_students  (GET)  - mirrors backend/get_students.c
# --------------------------------------------------------------------
@app.get("/cgi-bin/get_students")
def get_students():
    users = read_records(USERS_FILE, 5)
    scores = read_records(SCORES_FILE, 6)

    attempts, total_pct = {}, {}
    for uname, quizName, date, total, correct, percentage in scores:
        attempts[uname] = attempts.get(uname, 0) + 1
        total_pct[uname] = total_pct.get(uname, 0.0) + float(percentage)

    out = []
    for studentId, name, email, username, _pwd in users:
        a = attempts.get(username, 0)
        avg = round(total_pct.get(username, 0.0) / a, 2) if a > 0 else 0.0
        out.append({
            "studentId": studentId, "name": name, "email": email,
            "username": username, "attempts": a, "average": avg,
        })
    return out


# --------------------------------------------------------------------
# Serve the frontend itself (HTML/CSS/JS) from the same app/port so
# the "/cgi-bin/..." relative paths in script.js resolve correctly
# with zero frontend changes.
# --------------------------------------------------------------------
app.mount("/", StaticFiles(directory=FRONTEND_DIR, html=True), name="frontend")
