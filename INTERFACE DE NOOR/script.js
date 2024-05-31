// Hna firebase

import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";
import { getAnalytics } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-analytics.js";
import { getDatabase, ref, set } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

// hadi function ta3 navbar ta3na

const navbar = document.getElementById('navbar');
const navLinks = document.querySelectorAll('.nav-link');
const sections = document.querySelectorAll('section');
const menuBtn = document.getElementById('menuBtn');
const navUl = document.querySelector('nav ul');

window.addEventListener('scroll', () => {
    if (window.scrollY > 50) {
        navbar.classList.remove('navbar-transparent');
        navbar.classList.add('navbar-solid');
    } else {
        navbar.classList.remove('navbar-solid');
        navbar.classList.add('navbar-transparent');
    }

    sections.forEach(section => {
        if (window.scrollY >= section.offsetTop - navbar.offsetHeight) {
            navLinks.forEach(link => {
                link.classList.remove('active-link');
                if (link.getAttribute('href') === `#${section.id}`) {
                    link.classList.add('active-link');
                }
            }
              );
        }
    }
);
}
);

menuBtn.addEventListener('click', () => {
    navUl.classList.toggle('show');
});

//hna tfrd your own firebase infos
const firebaseConfig = {
    apiKey: "", // for privacy :3
    authDomain: "", // for privacy :3
    databaseURL: "", // for privacy :3
    projectId: "", // for privacy :3
    storageBucket: "", // for privacy :3
    messagingSenderId: "", // for privacy :3
    appId: "" // for privacy :3
  };

// initialize Firebase ta3na
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);
const db = getDatabase();

//hnaya win kayn la fonction ta3na 
function writeBraille(braille) {
    set(ref(db, 'text/'), { //REPLACE THE REF TO YOUR OWN!
        text: braille
    }
)
};

// hna the sending area

document.getElementById('importBtn').addEventListener('click', () => {
    const fileInput = document.getElementById('fileInput');
    if (fileInput.files.length === 0) {
        alert('Younes said choose a file >:(( !'); // MKASH FILE 
    }
 else 
{
        const file = fileInput.files[0];
        const reader = new FileReader();
        reader.onload = function (e) {
            var fileOutput = e.target.result;
            writeBraille(fileOutput); // hna yl79 llfirebase
        };
        reader.readAsText(file);
        alert('ENJOY :D!'); // sweet dm :3
    }
}
);
