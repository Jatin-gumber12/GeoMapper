#include <iostream>
#include <fstream>
#include "httplib.h"      // HTTP server library
#include "json.hpp"       // JSON library
#include "GraphFunctions.h"  // Custom graph functions

using namespace httplib;
using json = nlohmann::json;  // Alias for JSON

int main() {
    Server svr;

    // Serve HTML, CSS, and JS files
    svr.Get("/", [](const Request&, Response& res) {
        std::ifstream file("index.html");
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });

    // Handle route finding API request
    svr.Post("/find-route", [](const Request& req, Response& res) {
        try {
            json request_json = json::parse(req.body);

            std::string origin = request_json["origin"];
            std::string destination = request_json["destination"];
            std::string preference = request_json["preference"];

            std::string citiesFilename = "cities.csv";
            std::string routesFilename = "routes.csv";
            std::string outputFilename = "output.html";

            bool biPreference = (preference == "cheapest");

            Graph graph(citiesFilename, routesFilename);
            if (graph.getCity(origin) == nullptr || graph.getCity(destination) == nullptr) {
                res.status = 400;
                res.set_content(R"({"error": "Invalid city names"})", "application/json");
                return;
            }

            graph.Dijkstras(origin, biPreference);
            std::stack<Location*> cityStack = graph.cityStacker(destination);
            std::stack<Route*> routeStack = graph.routeStacker(destination, biPreference);

            outputGenerator(outputFilename.c_str(), cityStack, routeStack, biPreference);

            json response_json;
            response_json["message"] = "Route calculation complete. Check output.html";
            response_json["origin"] = origin;
            response_json["destination"] = destination;
            response_json["preference"] = preference;
            response_json["output_file"] = outputFilename;

            res.set_content(response_json.dump(), "application/json");
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(R"({"error": "Internal server error"})", "application/json");
        }
    });

    // Serve the generated output.html
    svr.Get("/output.html", [](const Request&, Response& res) {
        std::ifstream file("output.html");
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });

    std::cout << "Server running on http://localhost:5000\n";
    svr.listen("0.0.0.0", 5000);

    return 0;
}
