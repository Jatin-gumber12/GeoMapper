#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <stack>

#include "Location.h"

using namespace std; 

vector<Location*> locationParser(string filename, vector<Route*> routes){
	fstream locations(filename.c_str());

	string country;
	string city;
	string latitude;
	string longitude;

	vector<Location*> cities;
	Location* node;

	while(locations.good()){
		getline(locations, country, ',');
		getline(locations, city, ',');
		getline(locations, latitude, ',');
		getline(locations, longitude);
	

		node = new Location(country, city, atof(latitude.c_str()), atof(longitude.c_str()));

		vector<Route*>::iterator it = routes.begin();

		while(it != routes.end()){
			if((*it) -> originS.compare(node -> capital) == 0){
				(*it) -> origin = node;
				node -> routes.push_back((*it));
			}
			else if((*it) -> destinationS.compare(node -> capital) == 0){
				(*it) -> destination = node;
			}
			++it;
		}

		cities.push_back(node);

	}
	cout << "Cities Parsed from: " << filename << endl;
	return cities;
}




vector<Route*> routeParser(string filename){

	fstream routes(filename.c_str());

	string originS;
	string destinationS;

	Location* origin = new Location();
	Location* destination = new Location();

	string type;
	string time;
	string cost;
	string note;

	vector<Route*> allRoutes;
	Route* edge;

	while(routes.good()){
		getline(routes, originS, ',');
		getline(routes, destinationS, ',');
		getline(routes, type, ',');
		getline(routes, time, ',');
		getline(routes, cost, ',');
		getline(routes, note);



		edge = new Route(origin, destination, type, atof(time.c_str()), atof(cost.c_str()), note);
		edge -> destinationS = destinationS;
		edge -> originS = originS;

		allRoutes.push_back(edge);
	}

	cout << "Routes Parsed from: " << filename << endl;
	return allRoutes;
}



void outputGenerator(string filename, stack<Location*> cities, stack<Route*> routes, bool costOrTime) {
    ofstream output(filename.c_str());

    output << R"(<!DOCTYPE html>
<html>
<head>
    <title>Path Visualization</title>
    <style>
        #map { height: 100vh; width: 100vw; }
        #controls {
            position: absolute; top: 10px; left: 10px;
            z-index: 1000; background: rgba(255,255,255,0.9);
            padding: 10px; border-radius: 8px;
            font-family: Arial, sans-serif;
            box-shadow: 0 2px 10px rgba(0,0,0,0.2);
        }
        #controls button {
            background-color: #4285F4;
            color: white;
            border: none;
            padding: 8px 16px;
            margin: 0 5px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.3s;
        }
        #controls button:hover {
            background-color: #3367D6;
            transform: translateY(-1px);
        }
        #controls button:active {
            transform: translateY(1px);
        }
        #controls button:disabled {
            background-color: #cccccc;
            cursor: not-allowed;
        }
        #progress {
            margin-top: 10px;
            font-size: 14px;
            color: #333;
        }
        #progress img {
            vertical-align: middle;
            height: 20px;
            width: 20px;
            margin-right: 5px;
        }
    </style>
</head>
<body>
    <div id="map"></div>
    <div id="controls">
        <button id="startBtn" onclick='startAnimation()'>Start</button>
        <button id="stopBtn" onclick='stopAnimation()' disabled>Stop</button>
        <div id="progress">Ready to start</div>
    </div>

    <script src="http://maps.google.com/maps/api/js?sensor=false&callback=initMap" async defer></script>
    <script>
        let map;
        let planeMarker;
        let animationId;
        let currentPath = 0;
        const paths = [];
        const stopMarkers = [];
        
        function initMap() {
            map = new google.maps.Map(document.getElementById('map'), {
                center: {lat: 0, lng: 0},
                zoom: 2
            });
            
            // Create plane marker
            planeMarker = new google.maps.Marker({
                position: {lat: 0, lng: 0},
                map: map,
                icon: {
                    path: google.maps.SymbolPath.FORWARD_CLOSED_ARROW,
                    scale: 4,
                    fillColor: '#EA4335',
                    fillOpacity: 1,
                    strokeWeight: 0
                },
                visible: false
            });

    )";

    vector<Location*> allStops;
    while (!cities.empty()) {
        allStops.push_back(cities.top());
        cities.pop();
    }
    reverse(allStops.begin(), allStops.end());

    vector<Route*> allRoutes;
    while (!routes.empty()) {
        allRoutes.push_back(routes.top());
        routes.pop();
    }
    reverse(allRoutes.begin(), allRoutes.end());

    for (size_t i = 0; i < allStops.size(); ++i) {
        Location* stop = allStops[i];
        output << "stopMarkers.push(new google.maps.Marker({\n";
        output << "  position: {lat: " << stop->lat << ", lng: " << stop->lon << "},\n";
        output << "  map: map,\n";
        output << "  title: '" << stop->capital << ", " << stop->country << "',\n";
        output << "  icon: {\n";
        output << "    url: 'https://maps.google.com/mapfiles/ms/icons/"
               << (i == 0 ? "red" : (i == allStops.size() - 1 ? "green" : "blue")) << "-dot.png',\n";
        output << "    scaledSize: new google.maps.Size(32, 32)\n";
        output << "  }\n";
        output << "}));\n";
    }

    for (size_t i = 0; i < allStops.size() - 1; ++i) {
        Location* origin = allStops[i];
        Location* destination = allStops[i + 1];
        Route* route = allRoutes[i];

        float cost = route->cost;
        if (route->transport == "plane") cost /= MULTI;

        output << "paths.push({\n";
        output << "  polyline: new google.maps.Polyline({\n";
        output << "    path: [\n";
        output << "      {lat: " << origin->lat << ", lng: " << origin->lon << "},\n";
        output << "      {lat: " << destination->lat << ", lng: " << destination->lon << "}\n";
        output << "    ],\n";
        output << "    strokeColor: '#4285F4',\n";
        output << "    strokeOpacity: 0.7,\n";
        output << "    strokeWeight: 3,\n";
        output << "    map: map\n";
        output << "  }),\n";
        output << "  origin: {lat: " << origin->lat << ", lng: " << origin->lon << "},\n";
        output << "  destination: {lat: " << destination->lat << ", lng: " << destination->lon << "},\n";
        output << "  info: '<img src=\"https://maps.google.com/mapfiles/ms/icons/red-dot.png\"> " << origin->country << " <img src=\"https://maps.google.com/mapfiles/ms/icons/green-dot.png\"> " << destination->country << "'\n";
        output << "});\n";

        output << "google.maps.event.addListener(paths[" << i << "].polyline, 'click', function(event) {\n";
        output << "  alert(paths[" << i << "].info.replace(/<[^>]*>/g, ''));\n";
        output << "});\n";
    }

    output << R"(
            if (stopMarkers.length > 0) {
                const bounds = new google.maps.LatLngBounds();
                stopMarkers.forEach(marker => bounds.extend(marker.getPosition()));
                map.fitBounds(bounds);
            }

            updateProgress();
        }
        
        function updateProgress() {
            const progressDiv = document.getElementById('progress');
            if (paths.length > 0 && currentPath < paths.length) {
                progressDiv.innerHTML = `Route ${currentPath+1} of ${paths.length}<br>${paths[currentPath].info}`;
            }
        }
        
        function animatePlane(timestamp) {
            if (!window.startTime) window.startTime = timestamp;
            const elapsed = timestamp - window.startTime;
            const progress = Math.min(elapsed / 2000, 1);
            
            const path = paths[currentPath];
            const lat = path.origin.lat + progress * (path.destination.lat - path.origin.lat);
            const lng = path.origin.lng + progress * (path.destination.lng - path.origin.lng);
            
            planeMarker.setPosition({lat, lng});
            planeMarker.setVisible(true);
            
            const angle = Math.atan2(path.destination.lng - path.origin.lng, 
                                     path.destination.lat - path.origin.lat) * 180 / Math.PI;
            planeMarker.setIcon({
                path: google.maps.SymbolPath.FORWARD_CLOSED_ARROW,
                scale: 4,
                rotation: angle,
                fillColor: '#EA4335',
                fillOpacity: 1,
                strokeWeight: 0
            });
            
            if (progress < 1) {
                animationId = requestAnimationFrame(animatePlane);
            } else {
                window.startTime = undefined;
                currentPath++;
                if (currentPath < paths.length) {
                    updateProgress();
                    setTimeout(() => {
                        planeMarker.setVisible(false);
                        animationId = requestAnimationFrame(animatePlane);
                    }, 500);
                } else {
                    document.getElementById('startBtn').disabled = false;
                    document.getElementById('stopBtn').disabled = true;
                    planeMarker.setVisible(false);
                    updateProgress();
                }
            }
        }
        
        function startAnimation() {
            document.getElementById('startBtn').disabled = true;
            document.getElementById('stopBtn').disabled = false;
            
            if (currentPath >= paths.length) currentPath = 0;
            
            updateProgress();
            window.startTime = undefined;
            planeMarker.setPosition(paths[currentPath].origin);
            animationId = requestAnimationFrame(animatePlane);
        }
        
        function stopAnimation() {
            cancelAnimationFrame(animationId);
            document.getElementById('startBtn').disabled = false;
            document.getElementById('stopBtn').disabled = true;
            if (planeMarker) planeMarker.setVisible(false);
            updateProgress();
        }
        
        window.initMap = initMap;
    </script>
</body>
</html>)";

    output.close();
    cout << "Output File Generated: " << filename << endl;
}

#endif
