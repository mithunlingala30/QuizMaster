/* =========================================================
   QuizMaster - script.js
   Shared helpers loaded on every page:
     - QM.api()      talk to the C CGI backend
     - QM.auth       session-like login state (localStorage)
     - QM.toast()    notifications
     - QM.modal      confirmation dialogs
     - QM.initChrome() sidebar/nav wiring common to every page

   NOTE: the CGI programs themselves hold the real authority -
   they re-validate every request against the .txt files on the
   server. localStorage here only remembers *who is currently
   using the browser* between page loads (a "session-like" client
   state, since plain CGI has no built-in session store) - it is
   never trusted for scoring or admin actions.
   ========================================================= */

const QM = (() => {

  /* Change this if you deploy the compiled CGI binaries to a
     different alias in your web server config. */
  const API_BASE = "/cgi-bin/";

  /* ---------------- API layer ---------------- */
  async function api(endpoint, data) {
    const body = new URLSearchParams(data || {}).toString();
    const res = await fetch(API_BASE + endpoint, {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body
    });
    if (!res.ok) throw new Error("Server error (" + res.status + ")");
    return res.json();
  }

  async function apiGet(endpoint, params) {
    const qs = params ? "?" + new URLSearchParams(params).toString() : "";
    const res = await fetch(API_BASE + endpoint + qs);
    if (!res.ok) throw new Error("Server error (" + res.status + ")");
    return res.json();
  }

  /* ---------------- Session-like auth ---------------- */
  const auth = {
    setStudent(student) { localStorage.setItem("qm_student", JSON.stringify(student)); },
    getStudent() { try { return JSON.parse(localStorage.getItem("qm_student")); } catch { return null; } },
    setAdmin() { localStorage.setItem("qm_admin", "1"); },
    isAdmin() { return localStorage.getItem("qm_admin") === "1"; },
    logout() {
      localStorage.removeItem("qm_student");
      localStorage.removeItem("qm_admin");
      window.location.href = "login.html";
    },
    requireStudent() {
      const s = this.getStudent();
      if (!s) { window.location.href = "login.html"; return null; }
      return s;
    },
    requireAdmin() {
      if (!this.isAdmin()) { window.location.href = "admin-login.html"; return false; }
      return true;
    }
  };

  /* ---------------- Toast ---------------- */
  function toast(message, type = "default") {
    let stack = document.querySelector(".toast-stack");
    if (!stack) {
      stack = document.createElement("div");
      stack.className = "toast-stack";
      document.body.appendChild(stack);
    }
    const el = document.createElement("div");
    el.className = "toast " + type;
    el.textContent = message;
    stack.appendChild(el);
    setTimeout(() => el.remove(), 3800);
  }

  /* ---------------- Modal ---------------- */
  function confirmModal({ title, message, confirmLabel = "Confirm", danger = false, onConfirm }) {
    let overlay = document.getElementById("qm-confirm-modal");
    if (overlay) overlay.remove();
    overlay = document.createElement("div");
    overlay.className = "modal-overlay show";
    overlay.id = "qm-confirm-modal";
    overlay.innerHTML = `
      <div class="modal-box">
        <div class="m-icon ${danger ? "danger" : ""}">${danger ? "!" : "?"}</div>
        <h3>${title}</h3>
        <p>${message}</p>
        <div class="modal-actions">
          <button class="btn btn-ghost" data-act="cancel">Cancel</button>
          <button class="btn ${danger ? "btn-danger" : "btn-primary"}" data-act="ok">${confirmLabel}</button>
        </div>
      </div>`;
    document.body.appendChild(overlay);
    overlay.addEventListener("click", (e) => {
      if (e.target === overlay || e.target.dataset.act === "cancel") overlay.remove();
      if (e.target.dataset.act === "ok") { overlay.remove(); onConfirm && onConfirm(); }
    });
  }

  /* ---------------- Shared chrome: sidebar toggle + active link + logout wiring ---------------- */
  function initChrome() {
    document.querySelectorAll("[data-logout]").forEach(btn => {
      btn.addEventListener("click", () => auth.logout());
    });

    const menuToggle = document.querySelector(".menu-toggle");
    const sidebar = document.querySelector(".sidebar");
    if (menuToggle && sidebar) {
      menuToggle.addEventListener("click", () => sidebar.classList.toggle("open"));
    }

    const navToggle = document.querySelector(".nav-toggle");
    const navLinks = document.querySelector(".nav-links");
    if (navToggle && navLinks) {
      navToggle.addEventListener("click", () => navLinks.classList.toggle("open"));
    }

    const current = window.location.pathname.split("/").pop() || "index.html";
    document.querySelectorAll(".side-nav a").forEach(a => {
      if (a.getAttribute("href") === current) a.classList.add("active");
    });
  }

  function escapeHtml(str) {
    const div = document.createElement("div");
    div.textContent = str ?? "";
    return div.innerHTML;
  }

  document.addEventListener("DOMContentLoaded", initChrome);

  return { api, apiGet, auth, toast, confirmModal, escapeHtml };
})();
