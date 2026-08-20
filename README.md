<div align="center">

# 🎯 QuizMaster
### *High-Performance Online Quiz & Assessment Engine Built with Pure C & CGI*

[![C99](https://img.shields.io/badge/Language-C99-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C99)
[![Web](https://img.shields.io/badge/Frontend-HTML5%20%7C%20CSS3%20%7C%20JS-E34F26?style=for-the-badge&logo=html5&logoColor=white)](https://developer.mozilla.org/)
[![CGI](https://img.shields.io/badge/Architecture-CGI%20Binaries-blueviolet?style=for-the-badge)](https://en.wikipedia.org/wiki/Common_Gateway_Interface)
[![Storage](https://img.shields.io/badge/Data%20Store-Flat%20File%20(Pipe--Delimited)-success?style=for-the-badge)](https://en.wikipedia.org/wiki/Flat-file_database)
[![FastAPI](https://img.shields.io/badge/Dev%20Server-FastAPI%20%7C%20Uvicorn-009688?style=for-the-badge&logo=fastapi&logoColor=white)](https://fastapi.tiangolo.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=for-the-badge)](https://github.com)

<br>

**A full-stack, zero-external-database assessment platform demonstrating low-level systems programming, real-world C file handling, and browser-to-native CGI bridging.**

[Key Features](#-key-features) • [System Architecture](#-system-architecture) • [File Handling Deep-Dive](#-c-file-handling-implementation) • [Quick Start](#-quick-start--how-to-run) • [Project Structure](#-project-structure) • [Demo Credentials](#-demo-credentials)

---

</div>

## 📌 1. Project Overview

**QuizMaster** is an online examination and assessment management system created to demonstrate real-world systems programming concepts. Unlike conventional web applications that rely on heavy database engines (SQL/NoSQL) and modern runtime interpreters (Node/Python/PHP), QuizMaster is powered by **pure, compiled C programs operating as Common Gateway Interface (CGI) binaries** interacting directly with **pipe-delimited flat `.txt` files**.

### 🌟 Why This Project Stands Out (Hackathon Highlights)
* **Zero Heavy Dependencies:** No MySQL, Postgres, MongoDB, or Redis required.
* **Low-Level Native Performance:** Backend logic runs at compiled native C speeds.
* **Pure Systems-Level Persistence:** Complete CRUD engine implemented strictly with standard C library file-handling APIs (`<stdio.h>`).
* **Server-Side Truth:** All validation, authentication, scoring, and percentage calculations occur strictly server-side in C to prevent client tampering.
* **Dual Runtime Modes:** Includes native C CGI binaries for production/evaluations and an instant zero-config FastAPI dev-server for cross-platform live demonstrations.

---

## ✨ 2. Key Features

<table>
  <tr>
    <th width="50%">👨‍🎓 Student Portal</th>
    <th width="50%">👨‍💼 Admin & Management Portal</th>
  </tr>
  <tr>
    <td>
      <ul>
        <li><b>Self Registration & Auth:</b> Unique username validation with record scanning in C.</li>
        <li><b>Interactive Student Dashboard:</b> Overview of recent attempts, total quizzes taken, and average score.</li>
        <li><b>Timed Quiz Engine:</b> Dynamic question navigator, active status tracking, and countdown timer.</li>
        <li><b>Instant Server-Side Grading:</b> Automatic score evaluation, percentage calculation, and pass/fail analysis.</li>
        <li><b>Detailed Answer Review:</b> Breakdown of chosen answers vs. correct answers with explanations.</li>
        <li><b>Performance History:</b> Filterable log of past attempts stored permanently in <code>scores.txt</code>.</li>
      </ul>
    </td>
    <td>
      <ul>
        <li><b>Dedicated Secure Admin Gateway:</b> Isolated administrative authentication via <code>admin.txt</code>.</li>
        <li><b>Live Analytics Dashboard:</b> Real-time statistics, aggregate performance metrics, and CSS bar charts.</li>
        <li><b>Full Question Bank CRUD:</b>
          <ul>
            <li>Add new questions with multi-option inputs.</li>
            <li>In-place edit existing questions via temporary file swap.</li>
            <li>Delete questions with atomic file re-indexing.</li>
          </ul>
        </li>
        <li><b>Student Performance Registry:</b> Cross-table aggregation linking registered users with quiz scores.</li>
        <li><b>Comprehensive Result Search & Filter:</b> Search results by student, quiz title, date, or score tier.</li>
      </ul>
    </td>
  </tr>
</table>

---

## 🏗️ 3. System Architecture & Request Lifecycle

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           Client Browser                                │
│          (HTML5 / CSS3 / Vanilla JS - UI, State, Modals & Toast)        │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ HTTP Request (Fetch API)
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    Web / CGI Gateway Server                             │
│       (Apache mod_cgi / Python CGI Server / FastAPI Dev Server)         │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ Spawns C Binary Subprocess
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     Compiled Native C Programs                          │
│     (student_login, add_question, submit_quiz, get_results, etc.)       │
│                                                                         │
│  • Reads QUERY_STRING or stdin (CGI standard)                           │
│  • Parses payload via backend/common.h helpers                          │
│  • Performs file ops (fopen, fgets, fprintf, fputs, rename)             │
│  • Formats response & streams Content-Type: application/json to stdout   │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ Low-level Disk I/O
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    Flat File Data Store (data/*.txt)                    │
│   users.txt  │  admin.txt  │  questions.txt  │  scores.txt  │  temp.txt │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 💾 4. Data Storage & Schema Format

All data is structured as flat, human-readable, pipe-delimited (`|`) text files, allowing fast line-by-line scanning and constant-memory streaming:

| File | Purpose | Schema / Field Layout |
| :--- | :--- | :--- |
| `data/users.txt` | Student Records | `studentId \| name \| email \| username \| password` |
| `data/admin.txt` | Admin Credentials | `username \| password` |
| `data/questions.txt` | Question Bank | `id \| question \| optionA \| optionB \| optionC \| optionD \| correctAnswer \| category \| difficulty \| marks` |
| `data/scores.txt` | Quiz Submissions | `username \| quizName \| date \| totalQuestions \| correctAnswers \| percentage` |
| `data/temp.txt` | Atomic File Swaps | *Temporary buffer file used during update and delete operations* |

---

## ⚙️ 5. C File Handling Implementation

QuizMaster exercises all fundamental C file-handling operations required for low-level systems programming:

| Operation | C Standard Function | Implementation Details & Usage in Project |
| :--- | :--- | :--- |
| **File Opening** | `fopen(path, mode)` | Used in `"r"` (read), `"a"` (append), and `"w"` (overwrite/temp) modes across all 10 CGI programs. |
| **Safe Closing** | `fclose(fp)` | Deterministically called after every I/O transaction to prevent file lockups and descriptor leaks. |
| **Record Streaming** | `fgets(buffer, size, fp)` | Safely reads records line-by-line with bounded buffers to prevent buffer overflow vulnerabilities. |
| **Formatted Write** | `fprintf(fp, format, ...)` | Writes structured, pipe-delimited records in `student_register.c`, `add_question.c`, and `submit_quiz.c`. |
| **ID Parsing** | `fscanf(fp, ...)` / `sscanf(...)` | Extracts sequential numeric IDs and separates pipe-delimited tokens from text buffers. |
| **Direct Copying** | `fputs(str, fp)` | Streams unmodified lines through into `temp.txt` during editing or deletion cycles. |
| **Atomic Replace** | `remove()` & `rename()` | Standard industry pattern for flat-file record modification: writes output to `temp.txt`, deletes the original, and renames the temporary file into place. |

---

## 📂 6. Project Structure

```
QuizMaster/
│
├── frontend/                     # Modern Vanilla Web Interface
│   ├── css/
│   │   └── style.css             # Unified Responsive Design System & Dark Accents
│   ├── js/
│   │   └── script.js             # API Client, State Manager, Toasts & Modals
│   ├── index.html                # Project Landing & Entry Page
│   ├── login.html                # Student Authentication
│   ├── register.html             # Student Account Creation
│   ├── student-dashboard.html    # Student Overview & Performance
│   ├── quiz.html                 # Dynamic Timed Quiz Engine
│   ├── result.html               # Graded Scorecard & Review Breakdown
│   ├── my-results.html           # Historical Results for Student
│   ├── admin-login.html          # Admin Portal Gateway
│   ├── admin-dashboard.html      # Admin Live Metrics & Visual Charts
│   ├── manage-questions.html     # Question Bank Management Table
│   ├── add-question.html         # Question Creator Interface
│   ├── edit-question.html        # Question Editor Interface
│   ├── students.html             # Registered Student Directory & Statistics
│   └── admin-results.html        # Global Result Search & Filtering Hub
│
├── backend/                      # Pure C CGI Source Code
│   ├── common.h                  # Shared structs, CGI parser, JSON escape, path resolver
│   ├── student_register.c        # Handles student registration & duplicate checks
│   ├── student_login.c           # Student authentication against users.txt
│   ├── admin_login.c             # Admin authentication against admin.txt
│   ├── add_question.c            # Appends questions to questions.txt
│   ├── edit_question.c           # Atomic record modifier using temp file swap
│   ├── delete_question.c         # Record remover using temp file swap
│   ├── get_questions.c           # JSON question provider
│   ├── submit_quiz.c             # Server-side quiz evaluation and score recorder
│   ├── get_results.c             # Result queries with student/filter options
│   └── get_students.c            # User-Score join aggregation engine
│
├── data/                         # Persistent Flat File Database
│   ├── users.txt                 # Stored student credentials and profiles
│   ├── admin.txt                 # Admin authentication record
│   ├── questions.txt             # Preloaded question bank
│   └── scores.txt                # Logged quiz submissions
│
├── dev-server/                   # Instant Dev & Demo Server (FastAPI)
│   ├── app.py                    # Endpoint replica executing identical flat-file logic
│   └── requirements.txt          # Python dev dependencies (FastAPI, Uvicorn)
│
└── README.md                     # Comprehensive Project Documentation
```

---

## 🚀 7. Quick Start & How to Run

You can run QuizMaster using any of the following setups:

### 🔹 Option A: Instant Live Demo (Recommended for Windows / Fast Hackathon Demo)
Uses the lightweight FastAPI dev server (replicates the exact C CGI routes and reads/writes the same `data/*.txt` files):

```powershell
# 1. Navigate to dev-server directory
cd dev-server

# 2. Install dependencies
python -m pip install -r requirements.txt

# 3. Start the server
python -m uvicorn app:app --reload --port 8000
```
👉 Open your browser at **[http://localhost:8000/index.html](http://localhost:8000/index.html)**.

---

### 🔹 Option B: Native C CGI via Python Built-in Server (Linux / macOS / WSL)
Executes the actual compiled C binaries directly:

```bash
# 1. Prepare runtime directory structure
mkdir -p run/cgi-bin run/data
cp frontend/*.html run/
cp -r frontend/css frontend/js run/
cp data/*.txt run/data/

# 2. Compile all C backend programs into cgi-bin
cd backend
for f in *.c; do gcc -o "../run/cgi-bin/${f%.c}" "$f"; done
chmod +x ../run/cgi-bin/*
cd ..

# 3. Launch Python's CGI-capable server
cd run
python3 -m http.server 8000 --cgi
```
👉 Open your browser at **[http://localhost:8000/index.html](http://localhost:8000/index.html)**.

---

### 🔹 Option C: Production Apache Deployment
1. Compile backend programs into Apache's `cgi-bin` directory:
   ```bash
   cd backend
   for f in *.c; do gcc -o "/usr/lib/cgi-bin/${f%.c}" "$f"; done
   chmod +x /usr/lib/cgi-bin/*
   ```
2. Enable Apache CGI module and configure permissions:
   ```bash
   sudo a2enmod cgi
   sudo chmod -R 664 data/*.txt
   sudo chown -R www-data:www-data data/
   ```
3. Copy `frontend/` contents into your web document root (`/var/www/html/`).

---

## 🔑 8. Demo Credentials

The system comes pre-populated with test accounts and sample C programming questions:

| Portal | Username | Password | Role / Access |
| :--- | :--- | :--- | :--- |
| **Admin Portal** | `admin` | `admin123` | Full access to question bank, student records, and analytics. |
| **Student Portal** | `student01` | `pass123` | Active student account with sample historical results. |
| **Student Portal** | `student02` | `pass123` | Active student account. |
| **Student Portal** | `student03` | `pass123` | Active student account. |

*(New student accounts can also be created dynamically via the Registration page).*

---

## 🔒 9. Security & Robustness Highlights

* **Server-Side Grading Integrity:** Answers are graded strictly against `questions.txt` inside `submit_quiz.c`. The client only transmits selected option keys (`A`, `B`, `C`, `D`), preventing any inspect-element or client-side payload manipulation.
* **Data Sanitization:** Input values are escaped against JSON formatting breaks (`json_escape` in `common.h`) and HTML-escaped on render (`QM.escapeHtml`) to prevent injection flaws.
* **Isolated Admin Store:** Admin authentication is decoupled into an isolated `admin.txt` file and never exposed to client-side scripts.
* **Autonomous Path Resolution:** C programs utilize `/proc/self/exe` binary location resolution (`goto_own_directory()`) to ensure consistent relative file path targeting across different web server working directory configurations.

---

## 🔮 10. Future Roadmap

- [ ] Password hashing with `bcrypt` / `argon2` integration in C.
- [ ] Multi-category quiz partitioning & custom timer configurations.
- [ ] Anti-cheat tab-switch detection & fullscreen enforcement.
- [ ] Automated certificate generation (PDF output via C).
- [ ] CSV/Excel bulk export for administrative reports.

---

<div align="center">

Made with ❤️ for Hackathons & Systems Programming Excellence

</div>