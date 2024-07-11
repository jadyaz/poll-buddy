// Function to add options to the options container
function addOption() {
    var optionsContainer = document.getElementById("optionsContainer");
    var input = document.createElement("input");
    input.type = "text";
    input.name = "option[]";
    input.required = true;
    optionsContainer.appendChild(input);
}

// Function that handles poll creation form submission
function handlePollFormSubmission(pollForm) {
    pollForm.addEventListener('submit', function(event) {
        event.preventDefault();
        const formData = new FormData(pollForm);
        const options = formData.getAll('option[]');
        const data = {
            title: formData.get('pollTitle'), 
            options: options
        };

        fetch('/submit_poll', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
        }).then(response => response.json())
          .then(data => {
              window.location.href = data.url; // Redirect to the new poll
          }).catch((error) => {
              console.error('Error:', error);
          });
    });
}


// Function to check if user has already voted (using localStorage for simplicity)
function hasUserVoted(pollId) {
    const votedPolls = JSON.parse(localStorage.getItem('votedPolls')) || [];
    return votedPolls.includes(pollId);
}

// Function to mark poll as voted in localStorage
function markAsVoted(pollId) {
    const votedPolls = JSON.parse(localStorage.getItem('votedPolls')) || [];
    if (!votedPolls.includes(pollId)) {
        votedPolls.push(pollId);
        localStorage.setItem('votedPolls', JSON.stringify(votedPolls));
    }
}



document.addEventListener('DOMContentLoaded', function() {
    const pollForm = document.getElementById('pollForm');
    if (pollForm) {
        handlePollFormSubmission(pollForm);
    }

    const pollOptionsContainer = document.getElementById('pollOptions');
    // Check if pollOptions exists indicating we're on a poll page and not a form page
    if (pollOptionsContainer) {
        loadPollData();
    }

    const voteForm = document.getElementById('voteForm');
    if (voteForm) {
        setupVoteFormListener(voteForm);
    }
});

// This function loads poll data from the server and inserts it into the poll page.
function loadPollData() {
    const pollId = window.location.pathname.split('/poll/')[1];
    if (!pollId) return; // URL doesn't match expected pattern for a poll
    if (hasUserVoted(pollId)) {
        window.location.href = '/results/' + encodeURIComponent(pollId); // Redirect to a page indicating they've already voted
        return;
    }
    fetch('/api/poll/' + encodeURIComponent(pollId))
    .then(response => response.json())
    .then(data => {
        document.getElementById('pollTitle').textContent = 'Vote on ' + data.title;
        const pollOptionsContainer = document.getElementById('pollOptions');
        pollOptionsContainer.innerHTML = ''; // Clear any existing options
        data.options.forEach((option, index) => {
            const label = document.createElement('label');
            const radioButton = document.createElement('input');
            radioButton.type = 'radio';
            radioButton.name = 'pollOption';
            radioButton.value = option;
            radioButton.id = 'option' + index;

            label.appendChild(radioButton);
            label.appendChild(document.createTextNode(' ' + option));
            pollOptionsContainer.appendChild(label);
            pollOptionsContainer.appendChild(document.createElement('br'));
        });

        // Set the href for the results link
        document.getElementById('resultsLink').href = '/results/' + encodeURIComponent(pollId);
    })
    .catch(error => {
        console.error('Failed to load poll data:', error);
    });
}

//Function to submit the vote and redirect the user
function setupVoteFormListener(voteForm) {
    voteForm.addEventListener('submit', function(event) {
        event.preventDefault();
        const formData = new FormData(this);
        const selectedOption = formData.get('pollOption');
        const pollId = window.location.pathname.split('/poll/')[1];

        fetch('/vote/' + encodeURIComponent(pollId), {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ option: selectedOption })
        })
        .then(response => {
            if (!response.ok) {
                throw new Error(`Vote submission failed with status: ${response.status}`);
            }
            return response.json();
        })
        .then(jsonResponse => {
            console.log(jsonResponse.message); // Log the server's confirmation message.
            markAsVoted(pollId);
            // Redirect to the results page after a successful vote
            window.location.href = '/results/' + encodeURIComponent(pollId);
        })
        .catch(error => {
            console.error('Failed to submit vote:', error);
        });
    });
}

if (window.location.pathname.startsWith('/poll/')) {
    loadPollData();
}



function loadResultsData() {
    const pollId = window.location.pathname.split('/results/')[1];
    if (!pollId) return; 

    fetch('/api/results/' + encodeURIComponent(pollId))
    .then(response => response.json())
    .then(data => {
        document.getElementById('pollTitle').textContent = 'Votes for ' + data.title;
        const pollOptionsContainer = document.getElementById('pollOptions');
        pollOptionsContainer.innerHTML = ''; 
        for (const option in data.votes) {
            const voteCount = data.votes[option];
            const optionElement = document.createElement('div');
            optionElement.textContent = `${option}: ${voteCount}`;
            pollOptionsContainer.appendChild(optionElement);
        }
        const voteLink = document.getElementById("voteLink");
        const pollUrl = 'poll-buddy.com/poll/' + encodeURIComponent(pollId);
        voteLink.innerHTML = `Share this link with your voters: <a href="http://${pollUrl}" target="_blank">${pollUrl}</a>`;    
    })
    .catch(error => {
        console.error('Failed to load poll results:', error);
    });
}

if (window.location.pathname.startsWith('/results/')) {
    loadResultsData();
}