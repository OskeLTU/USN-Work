document.addEventListener('DOMContentLoaded', function() {
    const button = document.getElementById('js-button');
    const output = document.getElementById('js-output');
  
    if (button) {
      button.addEventListener('click', function() {
        output.textContent = "JavaScript is working! Button clicked.";
      });
    }
  });
  