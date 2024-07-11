#include "crow.h"
#include <fstream>
#include <random>
#include <unordered_set>
#include <sstream>


// Helper function to read the contents of a file and return as a string
std::string read_file_to_string(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + path);
    }
    buffer << file.rdbuf();
    return buffer.str();
}

//Index of already created Ids
std::unordered_set<int> ids;

//Helper function to generate a unique id for each poll
std::string generate_unique_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10000000, 99999999);

    int number;
    bool unique = false;
    
    while (!unique) {
        number = dis(gen);
        if (ids.find(number) == ids.end()) {
            unique = true;
            ids.insert(number);
        }
    }

    return "poll_" + std::to_string(number);
}


//our structure to hold the title, options, and votes of each poll
struct Poll {
    std::string title;
    std::vector<std::string> options;
    std::map<std::string, int> votes; 
    //add ip set
    Poll() = default; 

    Poll(const std::string& t, const std::vector<std::string>& opts)
        : title(t), options(opts) 
    {
        for (const auto& opt : options) {
            votes[opt] = 0; 
        }
    }
};


// Define a map to store all created polls with their unique identifiers
std::unordered_map<std::string, Poll> polls;

// Main function sets up the server and the routing
int main() {
    crow::SimpleApp app;  
  

    CROW_ROUTE(app, "/submit_poll").methods("POST"_method)
    ([&](const crow::request& req) {
        auto json_data = crow::json::load(req.body);
        //pulls json through crow

        if (!json_data || !json_data.has("title") || json_data["title"].t() != crow::json::type::String ||
            !json_data.has("options") || json_data["options"].t() != crow::json::type::List) {
            return crow::response(400, "Invalid JSON structure");
            //structure checks
        }

        std::string pollId = generate_unique_id();
        Poll newPoll(json_data["title"].s(), {}); 
        

        //add options to the votes map and sets the values to 0
        for (const auto& option : json_data["options"]) {
            if (option.t() != crow::json::type::String) {
                return crow::response(400, "Invalid option type");
            }
            std::string opt = option.s();
            newPoll.options.push_back(opt);
            newPoll.votes[opt] = 0; 
        }
        
        polls[pollId] = std::move(newPoll); 

        crow::json::wvalue response;
        response["pollId"] = pollId;
        response["url"] = "/poll/" + pollId;
        

        return crow::response{response};
    });

    // Serve the HTML poll page
    CROW_ROUTE(app, "/poll/<string>")
    ([](const std::string& pollId) {
        return crow::response(read_file_to_string("./templates/poll.html")); // Correct path for poll.html
    });

    //Endpoint to fetch json data for a poll
    CROW_ROUTE(app, "/api/poll/<string>")
    ([&](const std::string& pollId) {
        auto it = polls.find(pollId);
        if (it == polls.end()) {
            return crow::response(404, "Poll not found");
        }
        const Poll& poll = it->second;
        crow::json::wvalue response;
        response["title"] = poll.title;
        response["options"] = poll.options;
        return crow::response{response};
    });

    //route to the javascript
    CROW_ROUTE(app, "/views/<string>")
    ([](const crow::request& req, const std::string& filename){
            std::string path = "../src/views/" + filename;
            try {
                return crow::response(read_file_to_string(path));
            } catch (const std::runtime_error& e) {
                return crow::response(404, "File not found: " + std::string(e.what()));
            }
    });

    // Serve the HTML results page
     CROW_ROUTE(app, "/results/<string>")
    ([](const std::string& pollId) {
        return crow::response(read_file_to_string("./templates/results.html")); // Correct path for results.html
    });

    // Return JSON data for a specified poll
    CROW_ROUTE(app, "/api/results/<string>").methods("GET"_method)
    ([&](const std::string& pollId) {
        auto it = polls.find(pollId);
        if (it == polls.end()) {
            return crow::response(404, "Poll not found");
        }
        
        const Poll& poll = it->second;
        crow::json::wvalue response;
        response["title"] = poll.title;
        response["votes"] = crow::json::wvalue(crow::json::wvalue::object());
        
        for (const auto& [option, count]: poll.votes) {
            response["votes"][option] = count;
        }

        return crow::response{response};
    });

    // Post a user's vote for a specified poll
    CROW_ROUTE(app, "/vote/<string>").methods(crow::HTTPMethod::Post)
    ([&](const crow::request& req, const std::string& pollId) {
        auto voted_option = crow::json::load(req.body);

        if (!voted_option || !voted_option.has("option") || voted_option["option"].t() != crow::json::type::String) {
            CROW_LOG_ERROR << "Vote failed to parse or invalid option type";
            return crow::response(400, "Invalid request");
        }

        auto poll_iter = polls.find(pollId);
        if (poll_iter == polls.end()) {
            CROW_LOG_ERROR << "Poll not found with id: " << pollId;
            return crow::response(404, "Poll not found");
        }

        Poll& poll = poll_iter->second;
        const std::string& option = voted_option["option"].s();
        auto vote_iter = poll.votes.find(option);
        if (vote_iter == poll.votes.end()) {
            CROW_LOG_ERROR << "Option not found in poll: " << option;
            return crow::response(400, "Invalid option");
        }

        vote_iter->second++;
        
        crow::json::wvalue response;
        response["message"] = "Vote successfully recorded for option: " + option;
        return crow::response{response};
    });

    // Serve the submission page for creating new polls
    CROW_ROUTE(app, "/")
    ([]() {
        return crow::response(read_file_to_string("./templates/index.html")); // Correct path for index.html
    });

    app.port(8080).multithreaded().run();

    return 0;
}