#include <iostream>
#include "httplib.h"      // For HTTP server
#include "json.hpp" // JSON library
#include "GraphFunctions.h"  // Your custom graph functions

using namespace httplib;
using json = nlohmann::json; // Alias for JSON

int main() {
    Server svr;

    // API Endpoint: Find Route
    svr.Post("/find-route", [](const Request& req, Response& res) {
        try {
            // ✅ FIX 1: Correct JSON Parsing
            json request_json = json::parse(req.body);

            std::string origin = request_json["origin"];
            std::string destination = request_json["destination"];
            std::string preference = request_json["preference"];

            // Default input files (automated)
            std::string citiesFilename = "cities.csv";
            std::string routesFilename = "routes.csv";
            std::string outputFilename = "output.html";

            // ✅ FIX 2: Validate preference
            bool biPreference;
            if (preference == "cheapest") {
                biPreference = true;
            } else if (preference == "fastest") {
                biPreference = false;
            } else {
                res.status = 400;
                res.set_content(R"({"error": "Invalid preference. Use 'fastest' or 'cheapest'"})", "application/json");
                return;
            }
 
            // Load Graph and Validate Cities
            Graph graph(citiesFilename, routesFilename);
            if (graph.getCity(origin) == nullptr || graph.getCity(destination) == nullptr) {
                res.status = 400;
                res.set_content(R"({"error": "Invalid city names"})", "application/json");
                return;
            }

            // Run Dijkstra's Algorithm
            graph.Dijkstras(origin, biPreference);
            std::stack<Location*> cityStack = graph.cityStacker(destination);
            std::stack<Route*> routeStack = graph.routeStacker(destination, biPreference);

            // Generate Output File
            outputGenerator(outputFilename.c_str(), cityStack, routeStack, biPreference);

            // ✅ FIX 3: Create Proper JSON Response
            json response_json;
            response_json["message"] = "Route calculation complete. Check output.html";
            response_json["origin"] = origin;
            response_json["destination"] = destination;
            response_json["preference"] = preference;
            response_json["output_file"] = outputFilename;

            res.set_content(response_json.dump(), "application/json"); // Send JSON response
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(R"({"error": "Internal server error"})", "application/json");
        }
    });

    std::cout << "Server running on http://localhost:5000\n";
    svr.listen("0.0.0.0", 5000);  // Start server on port 5000

    return 0;
}
