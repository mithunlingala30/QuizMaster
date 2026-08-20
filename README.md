# QuizMaster — Online Quiz Management System

A full-stack online quiz platform built to demonstrate **C file handling** in a
real, working web application: HTML/CSS/JS on the frontend, plain **C
programs run as CGI scripts** on the backend, and **flat `.txt` files** as
the entire data store — no database, no PHP/Python/Node/Java, no Firebase.

---

## 1. Project Introduction

QuizMaster lets students register, log in, attempt multiple-choice quizzes,
and immediately see a graded result with a full answer breakdown. An
administrator can log in separately to add, edit, and delete questions, and
to review every student's performance. Every piece of data — accounts,
questions, and scores — is stored and manipulated using the standard C
`<stdio.h>` file functions (`fopen`, `fclose`, `fread`, `fwrite`, `fprintf`,
`fscanf`, `fgets`, `fputs`, `fseek`, `rewind`).

## 2. Objectives

- Demonstrate practical, real-world use of C file handling (not toy examples).
- Build a CGI bridge between a browser frontend and native C programs.
- Implement full CRUD (create, read, update, delete) on flat files.
- Calculate and store quiz scores entirely on the server, never trusting the client.
- Produce a project simple enough for a B.Tech student to compile, run, and explain in a viva.

## 3. Features

**Student:** registration, login, quiz selection, timed quiz attempt with a
question navigator, instant scoring, detailed answer review, and a personal
results history.

**Admin:** secure separate login, dashboard with live statistics and charts,
add/edit/delete questions, view all registered students with computed
performance, and search/filter/sort all quiz results.

## 4. Technologies Used

| Layer            | Technology                                   |
|-------------------|-----------------------------------------------|
| Frontend          | HTML5, CSS3, vanilla JavaScript (no framework) |
| Backend           | C (C99), compiled as CGI executables          |
| Web/CGI server    | Apache HTTP Server with `mod_cgi` (or any CGI-capable server) |
| Data storage      | Pipe-delimited `.txt` files, read/written with C file handling |

## 5. System Architecture

```
Browser (HTML/CSS/JS)
        │  fetch() → application/x-www-form-urlencoded / query string
        ▼
Web server (Apache) → /cgi-bin/
        │  runs the compiled C binary as a subprocess
        ▼
C CGI program (e.g. student_login)
        │  fopen()/fread()/fwrite() on data/*.txt
        ▼
Flat file data store (data/users.txt, questions.txt, scores.txt, admin.txt)
        │
        ▼
C program prints a JSON string to stdout
        │
        ▼
Web server relays stdout back as the HTTP response
        │
        ▼
Browser JS parses the JSON and updates the page
```

## 6. Frontend Architecture

Fourteen static HTML pages share one stylesheet (`css/style.css`) and one
script (`js/script.js`):

- `index.html` — landing page
- `register.html`, `login.html` — student auth
- `student-dashboard.html` — student home, stats, recent results
- `quiz.html` — quiz selection **and** the timed quiz attempt UI (two views in one page)
- `result.html` — score summary + toggleable detailed answer review
- `my-results.html` — student's full quiz history
- `admin-login.html` — admin auth
- `admin-dashboard.html` — admin stats + CSS-based bar charts
- `manage-questions.html`, `add-question.html`, `edit-question.html` — question CRUD
- `students.html` — registered students + computed performance
- `admin-results.html` — all results with search/filter/sort

`js/script.js` centralizes API calls (`QM.api` / `QM.apiGet`), a
localStorage-based "session-like" login state (`QM.auth`), toasts, and a
reusable confirmation modal, so every page only contains the logic specific
to itself.

## 7. C Backend Architecture

Each backend file is a standalone CGI program compiled to its own
executable. `backend/common.h` centralizes the shared `struct` definitions
and helper functions (CGI form parsing, JSON escaping, file-line splitting)
so the file-handling logic isn't duplicated ten times:

| Program              | Purpose                                              |
|-----------------------|-------------------------------------------------------|
| `student_register.c`  | Validates + appends a new student to `users.txt`      |
| `student_login.c`     | Authenticates a student against `users.txt`           |
| `admin_login.c`       | Authenticates the admin against `admin.txt`           |
| `add_question.c`      | Appends a new question to `questions.txt`             |
| `edit_question.c`     | Rewrites `questions.txt` with one record updated       |
| `delete_question.c`   | Rewrites `questions.txt` with one record removed        |
| `get_questions.c`     | Reads and returns all questions as JSON                |
| `submit_quiz.c`       | Grades answers against `questions.txt`, appends `scores.txt` |
| `get_results.c`       | Reads and returns quiz results (optionally filtered)    |
| `get_students.c`      | Joins `users.txt` and `scores.txt` into a student report |

## 8. File Handling Implementation

This project deliberately exercises every file mode and function requested
for the assignment:

| Function      | Where it's used                                                        |
|----------------|--------------------------------------------------------------------------|
| `fopen()`      | Every program — opens `.txt` files in `"r"`, `"a"`, or `"w"` mode        |
| `fclose()`     | Immediately after each read/write block                                  |
| `fprintf()`    | Writing pipe-delimited records (`add_question.c`, `student_register.c`, `submit_quiz.c`) |
| `fscanf()`     | Extracting a leading numeric ID from a line (`add_question.c`, `edit_question.c`) |
| `fgets()`      | Line-by-line reads of every data file                                     |
| `fputs()`      | Copying unmodified lines through during update/delete (`edit_question.c`, `delete_question.c`) |
| `sscanf()`     | Parsing fields out of a buffered line                                     |
| `remove()` / `rename()` | The classic "update a text file" pattern: write a full replacement to `temp.txt`, delete the original, rename the temp file into place |

**File modes demonstrated:** `"r"` (read-only scans for auth/reporting),
`"a"` (append-only writes for registration/new questions/new scores), and
`"w"` (used on `temp.txt` while rebuilding `questions.txt` during edit/delete).

## 9. File Structure

```
QuizMaster/
├── frontend/
│   ├── index.html, login.html, register.html, student-dashboard.html,
│   │   quiz.html, result.html, my-results.html, admin-login.html,
│   │   admin-dashboard.html, manage-questions.html, add-question.html,
│   │   edit-question.html, students.html, admin-results.html
│   ├── css/style.css
│   └── js/script.js
├── backend/
│   ├── common.h
│   ├── student_login.c, student_register.c, admin_login.c
│   ├── add_question.c, edit_question.c, delete_question.c
│   ├── get_questions.c, submit_quiz.c, get_results.c, get_students.c
├── data/
│   ├── users.txt, admin.txt, questions.txt, scores.txt, temp.txt
└── README.md
```

## 10. Data Formats

All files are pipe (`|`) delimited, one record per line.

```
data/users.txt      studentId|name|email|username|password
data/admin.txt      username|password
data/questions.txt  id|question|optionA|optionB|optionC|optionD|correctAnswer|category|difficulty|marks
data/scores.txt     username|quizName|date|totalQuestions|correctAnswers|percentage
```

## 11. How CGI Works Here

The browser sends a normal HTTP request (`GET` with a query string, or
`POST` with an `application/x-www-form-urlencoded` body) to a URL under
`/cgi-bin/`. Apache runs the matching compiled C binary as a child process:

- **GET** parameters arrive in the `QUERY_STRING` environment variable.
- **POST** bodies arrive on `stdin`, with their length in the
  `CONTENT_LENGTH` environment variable.
- Every program's *first* output must be an HTTP header block, so each one
  starts with `print_json_header()` → `Content-Type: application/json\r\n\r\n`.
- Everything printed after that becomes the HTTP response body — in this
  project, a JSON string the frontend `fetch()` call parses directly.

## 12. How to Compile the C Programs

From inside `backend/`:

```bash
gcc -o student_register.cgi student_register.c
gcc -o student_login.cgi    student_login.c
gcc -o admin_login.cgi      admin_login.c
gcc -o add_question.cgi     add_question.c
gcc -o edit_question.cgi    edit_question.c
gcc -o delete_question.cgi  delete_question.c
gcc -o get_questions.cgi    get_questions.c
gcc -o submit_quiz.cgi      submit_quiz.c
gcc -o get_results.cgi      get_results.c
gcc -o get_students.cgi     get_students.c
```

Or compile all at once:

```bash
cd backend && for f in *.c; do gcc -o "${f%.c}.cgi" "$f"; done
```

Every program has already been compiled and smoke-tested against the sample
data in this project (registration, login, add/edit/delete question, quiz
submission and scoring, and reporting all verified end-to-end) — including a
real run through both Apache-style CGI and Python's built-in test server
(see Section 14A below).

**A note on file paths:** `common.h` defines the data files as `../data/...`
(relative paths). At the top of every `main()`, `goto_own_directory()` uses
`/proc/self/exe` to resolve the compiled binary's own real location and
`chdir()`s there first, so `../data/...` always resolves correctly and
points at the project's `data/` folder — regardless of which directory the
web server itself was launched from, or what working directory it hands to
the CGI process. This matters because different CGI-capable servers behave
differently here (Apache's `mod_cgi` changes into the script's directory
automatically; Python's simple test server does not).

## 13. How to Configure the Web Server (Apache example)

1. Copy the compiled `.cgi` binaries into Apache's `cgi-bin` directory
   (e.g. `/usr/lib/cgi-bin/` on Debian/Ubuntu), **renaming each to match the
   endpoint names used in `js/script.js`** — e.g. `student_login.cgi` should
   be reachable as `/cgi-bin/student_login`. The simplest approach is to
   enable `mod_cgid`/`mod_cgi` and drop the binaries in without a file
   extension, e.g. `student_login` instead of `student_login.cgi`.
2. Make sure the `data/` folder is writable by the user Apache runs as
   (`www-data` on most distros): `chmod -R 664 data/*.txt && chown -R www-data data/`.
3. Because `common.h` uses relative paths (`../data/users.txt`), the CGI
   binaries must be run from a working directory one level above `data/` —
   in practice this means placing `cgi-bin/` and `data/` as siblings, matching
   this project's folder layout, or adjusting the `#define ..._FILE` paths
   in `common.h` to absolute paths for your server.
4. Serve `frontend/` as the site's document root (e.g. Apache's `htdocs` or
   `/var/www/html`).

## 14. How to Run the Project

> **Two ways to run this, for different purposes:**
> - **`backend/` (C/CGI)** — this is the real project. It's what demonstrates
>   C file handling and what you submit/present/explain in your viva.
> - **`dev-server/` (FastAPI)** — a Python stand-in that exposes the exact
>   same routes, request fields, and JSON responses as the C programs, and
>   reads/writes the exact same `data/*.txt` files. It exists purely so you
>   can click through the whole frontend on Windows without installing gcc
>   or WSL — e.g. while designing pages, or demoing the UI quickly. **It is
>   not a replacement for the C backend and should not be presented as the
>   project itself.** Data is fully compatible either way: register a
>   student via one backend, log in against the other, no conversion needed.

### 14A. Fastest option for testing on your own laptop (no Apache install)

This is the quickest way to see the whole thing working, verified end-to-end:

```bash
# 1. Build one folder that mirrors the deployed layout
mkdir -p run/cgi-bin run/data
cp frontend/*.html run/
cp -r frontend/css frontend/js run/
cp data/*.txt run/data/

# 2. Compile every backend program straight into cgi-bin/
cd backend
for f in *.c; do gcc -o ../run/cgi-bin/${f%.c} "$f"; done
chmod +x ../run/cgi-bin/*
cd ..

# 3. Start Python's built-in CGI-capable web server
cd run
python3 -m http.server 8000 --cgi
```

Then open **http://localhost:8000/index.html** in a browser. Login, quiz
attempt, admin question management, etc. all work against the real C
backend and the real `.txt` files in `run/data/`.

(Requires only `gcc` and `python3`, both already on most Linux/macOS
systems and available in WSL on Windows.)

### 14C. Fastest option on Windows without WSL (FastAPI dev-server)

If you're on Windows and don't want to set up WSL just to click through the
UI, run the FastAPI stand-in instead. **Remember: this is for testing only —
your actual submission is still the C backend in `backend/`.**

```powershell
cd dev-server
python -m pip install -r requirements.txt
python -m uvicorn app:app --reload --port 8000
```

Then open **http://localhost:8000/index.html**. In VS Code, tasks
"4. [DEV ONLY] Install FastAPI mock-backend deps" and "5. [DEV ONLY] Run
FastAPI mock backend" (Terminal > Run Task) do exactly this for you.

### 14B. Production-style option (Apache)

1. Compile the backend (`Section 12`).
2. Place the compiled binaries in your server's `cgi-bin` and the data files
   next to them as described in `Section 13`.
3. Serve `frontend/` as your web root and open `index.html` in a browser.
4. Register a student (or use a demo account below), log in, and take the
   sample "C Programming Basics" quiz.
5. Log in as admin separately to manage questions and view results.

## 15. Sample Login Credentials

| Role    | Username  | Password  |
|---------|-----------|-----------|
| Admin   | `admin`   | `admin123`|
| Student | `student01` | `pass123` |
| Student | `student02` | `pass123` |
| Student | `student03` | `pass123` |

10 sample C programming questions covering variables, data types, operators,
conditionals, loops, functions, arrays, pointers, structures, and file
handling are pre-loaded in `data/questions.txt`.

## 16. Future Enhancements

- Replace plain-text passwords with a hashed scheme (see security note below).
- Move from flat files to a lightweight embedded database (e.g. SQLite) while keeping the same C backend structure.
- Add per-quiz grouping so multiple distinct quizzes can be managed independently instead of one shared question bank.
- Add real server-side sessions/cookies instead of the client-side "session-like" localStorage state.
- Add negative marking and partial-credit scoring options.

---

## Why C File Handling?

This project exists specifically to put every core C file-handling operation
to real use, not as an isolated exercise but as the actual persistence layer
of a working application:

- **File creation** — the first `fopen(path, "a")` call on a fresh install creates `users.txt`/`questions.txt`/`scores.txt` if they don't already exist.
- **File opening** — every CGI program opens the exact file it needs, in the mode appropriate to what it's doing (`"r"` to read, `"a"` to append, `"w"` to rewrite).
- **File reading** — `fgets()` streams each `.txt` file line by line for login checks, listing questions, and building reports.
- **File writing** — `fprintf()` writes new pipe-delimited records for registrations, new questions, and new scores.
- **File appending** — registration, question creation, and quiz submission all use `"a"` mode so existing records are never disturbed.
- **File updating** — `edit_question.c` shows the standard C pattern for "editing a line" in a flat file: read the original, write a corrected copy to a temp file, then `remove()` + `rename()` to swap it into place.
- **File deletion** — `delete_question.c` uses the same read/rewrite/rename pattern, simply omitting the target record instead of replacing it.
- **Record searching** — `student_login.c`, `admin_login.c`, and `student_register.c`'s uniqueness check all scan a file line-by-line looking for a matching field.
- **Record modification** — `edit_question.c` demonstrates modifying one specific record identified by its ID while leaving every other record byte-for-byte untouched.

## Security Notes

- Admin credentials live only in `data/admin.txt` on the server and are
  never referenced by any frontend HTML/JS file.
- All login and registration checks happen in the C backend — the frontend's
  client-side validation is a convenience layer only, not a security boundary.
- **Quiz scores are always calculated server-side.** `submit_quiz.c` re-reads
  the answer key from `questions.txt` itself; the browser only ever sends
  which letter the student picked, never whether it was right.
- User-supplied text is HTML-escaped in the browser (`QM.escapeHtml`) before
  being inserted into the page, and quote/backslash-escaped in C
  (`json_escape`) before being embedded in JSON responses.
- **For this college demonstration, passwords are stored as plain text** in
  `users.txt`/`admin.txt` to keep the file-handling logic easy to read and
  explain. A production system should never do this — it should hash
  passwords (e.g. with bcrypt or a salted SHA-256) before writing them to
  disk and compare hashes on login instead of raw strings.
#   Q u i z M a s t e r  
 