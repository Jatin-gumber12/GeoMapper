#include <iostream>
#include <fstream>
#include "httplib.h"
#include "json.hpp"
#include "GraphFunctions.h"

using namespace httplib;
using json = nlohmann::json;

int main() {
    Server svr;

    svr.Get("/", [](const Request&, Response& res) {
        std::ifstream file("home.html");
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });

    svr.Get("/index.html", [](const Request&, Response& res) {
        std::ifstream file("index.html");  // Make sure index.html is in the same folder
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found: index.html", "text/plain");
        }
    });


    svr.Get(R"(/(.*))", [](const Request& req, Response& res) {
        std::string file_path = req.path.substr(1); // remove leading slash
        std::ifstream file(file_path, std::ios::binary);
    
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
     
            // Basic content-type detection
            std::string ext = file_path.substr(file_path.find_last_of('.') + 1);
            std::string mime = "text/plain";
            if (ext == "html") mime = "text/html";
            else if (ext == "css") mime = "text/css";
            else if (ext == "js") mime = "application/javascript";
            else if (ext == "webm") mime = "video/webm";
            else if (ext == "mp4") mime = "video/mp4";
    
            res.set_content(buffer.str(), mime);
        } else {
            res.status = 404;
            res.set_content("File not found: " + file_path, "text/plain");
        }
    });
    
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
