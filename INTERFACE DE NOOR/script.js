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
            });
        }
    });
});

menuBtn.addEventListener('click', () => {
    navUl.classList.toggle('show');
});

document.getElementById('importBtn').addEventListener('click', () => {
    const fileInput = document.getElementById('fileInput');
    if (fileInput.files.length === 0) {
        alert('Younes said choose a file >:(( !');
    } else {
        const file = fileInput.files[0];
        const reader = new FileReader();
        reader.onload = function(e) {
            const fileContent = e.target.result;
            // Send fileContent to ESP32 server

            fetch('http://your-local-server-address', {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain',
                },
                body: fileContent
            }).then(response => {
                if (response.ok) {
                    alert('File successfully sent to ESP32!');
                } else {
                    alert('Failed to send file to ESP32.');
                }
            }).catch(error => {
                console.error('Error:', error);
                alert('An error occurred while sending the file.');
            });
        };
        reader.readAsText(file);
    }
});
