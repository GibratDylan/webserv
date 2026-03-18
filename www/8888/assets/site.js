(function () {
  async function fetchJSON(url) {
    const res = await fetch(url, { headers: { Accept: "application/json" } });
    if (!res.ok) throw new Error("HTTP " + res.status);
    return await res.json();
  }

  function setWhoami(text, loggedIn) {
    const who = document.getElementById("whoami");
    if (who) who.textContent = text;
    const logout = document.getElementById("logoutLink");
    if (logout) logout.hidden = !loggedIn;
  }

  async function initWhoami() {
    try {
      const data = await fetchJSON("/cgi-bin/whoami.py");
      if (data && data.loggedIn) setWhoami("Connecté: " + data.username, true);
      else setWhoami("Non connecté", false);
    } catch (_) {
      setWhoami("Non connecté", false);
    }
  }

  async function initUploadsList() {
    const target = document.getElementById("uploadsList");
    if (!target) return;
    try {
      const data = await fetchJSON("/cgi-bin/uploads_list.py");
      const items = (data && data.files) || [];
      if (!items.length) {
        target.textContent = "(aucun fichier)";
        return;
      }
      const ul = document.createElement("ul");
      items.forEach((f) => {
        const li = document.createElement("li");
        const a = document.createElement("a");
        a.href = "/uploads/" + encodeURIComponent(f.name);
        a.textContent = f.name + " (" + f.size + " o)";
        li.appendChild(a);
        ul.appendChild(li);
      });
      target.replaceChildren(ul);
    } catch (e) {
      target.textContent = "(impossible de charger la liste)";
    }
  }

  async function initUploadForm() {
    const form = document.getElementById("uploadForm");
    if (!form) return;
    const input = document.getElementById("file");
    const status = document.getElementById("uploadStatus");
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      if (!input || !input.files || !input.files.length) {
        if (status) status.textContent = "Choisis un fichier.";
        return;
      }
      const file = input.files[0];
      const url = "/" + encodeURIComponent(file.name);
      if (status) status.textContent = "Upload en cours…";
      try {
        const res = await fetch(url, {
          method: "POST",
          body: file,
          headers: {
            "Content-Type": file.type || "application/octet-stream",
          },
        });
        if (res.status === 201) {
          if (status) status.textContent = "OK: /uploads/" + file.name;
          await initUploadsList();
        } else {
          const text = await res.text().catch(() => "");
          if (status) status.textContent = "Erreur HTTP " + res.status + (text ? ": " + text : "");
        }
      } catch (err) {
        if (status) status.textContent = "Erreur réseau";
      }
    });
  }

  document.addEventListener("DOMContentLoaded", function () {
    initWhoami();
    initUploadsList();
    initUploadForm();
  });
})();
