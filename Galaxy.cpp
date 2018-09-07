#include "galaxy.h"

using namespace std;

//**********************************************START OF READER CLASS**********************************************//

/* 
Precondition: None
Postcondition: None
Dumps out the time schedule dump from the conduits.txt
*/
void Reader::timeScheduleDump()
{
	for (auto const &ent1 : travelTimes) { //Iterates through the map<string, map<string, int> 
		auto const &outerKey = ent1.first;
		auto const &inner_map = ent1.second;
		for (auto const &ent2 : inner_map) {
			auto const &innerKey = ent2.first;
			auto const &innerVal = ent2.second;
			cerr << "First: " << outerKey << ", Second: " << innerKey << ", Weight: " << innerVal << endl;
		}
	}
}

/*
Precondition: None
Postcondition: None
Creates the time schedule. 
Creates the graph while validating with the time schedule. 
Returns the graph.
*/
Galaxy * Reader::load()
{
	createTimeSchedule();
	createGraph();
	return galaxy;
}

/*
Precondition: None
Postcondition: None
Creates time schedule from conduits. Utilized for validating the creation
of the graph. 
*/
void Reader::createTimeSchedule()
{
	string startPlanet = "";
	string endPlanet = "";
	string weight;
	int tabCount = 0;
	char letter;
	while ((letter = inFile.get())) {
		if (letter == '\t') {
			tabCount++;
			continue;
		}
		if (letter == '\n') {
			tabCount = 0;
			travelTimes[endPlanet][startPlanet] = stoi(weight);
			travelTimes[startPlanet][endPlanet] = stoi(weight);

			startPlanet = "";
			endPlanet = "";
			weight = "";
			continue;
		}
		if (tabCount == 0) { //Get the start planet
			startPlanet.push_back(letter);
		}
		else if (tabCount == 1) { //Get the end planet
			endPlanet.push_back(letter);
		}
		else if (tabCount == 2) { //Get the weight
			weight.push_back(letter);
		}
		if (letter == EOF) break;
	}
	inFile.close();
}

/*
Precondition: None
Postcondition: Returns a bool
Updates the current leg information.
*/
bool Reader::get_record()
{
	if (route.eof()) { //end of file
		route.close();
		return false;
	}
	else {
		getline(route, currentLeg);
		while (currentLeg.empty() || currentLeg[0] == '#' || currentLeg[0] == '\n') {
			getline(route, currentLeg); 
		}
	}
	return true;
}

/*
Precondition: None
Postcondition: Returns a bool
Checks the current leg information and validates the current leg information with the previos leg information. 
*/
bool Reader::validate()
{
	int hours;
	if (!previous_destination_planet) { //Special case. First departure. 
		return true;
	}
	else {
		if (ship_id != previous_ship_id) { //Start of another ship's route.  
			return true;
		}
		else {
			hours = travelTimes[departure_planet->name][destination_planet->name];
			//Checks if it takes the right amount of time to travel from planet A to planet B 
			if (!(departure_time + hours == arrival_time)) { 
				return false;
			}
			if (previous_arrival_time + MIN_LAYOVER_TIME > departure_time) { //Checks that the minimum layover time has been waited. 
				return false;
			}
			if (previous_destination_planet != departure_planet) { //Should leave from the same planet the ship arrived at. 
				return false;
			}
		}
	}
	return true;
}

/*
Precondition: None
Postcondition: creates graph
Parses the input file and creates the graph based on the leg information passed in.
Validates with the conduit.txt file. 
*/
void Reader::createGraph()
{
	//Edge* edge;
	string startPlanet, destPlanet, ship;
	int dept, arrival;
	while (get_record()) {
		//Parsing the information since each element is separated by a tab key. 
		ship = currentLeg.substr(0, currentLeg.find('\t'));
		currentLeg = currentLeg.substr(currentLeg.find('\t') + 1);
		startPlanet = currentLeg.substr(0, currentLeg.find('\t'));
		currentLeg = currentLeg.substr(currentLeg.find('\t') + 1);
		dept = stoi(currentLeg.substr(0, currentLeg.find('\t')));
		currentLeg = currentLeg.substr(currentLeg.find('\t') + 1);
		destPlanet = currentLeg.substr(0, currentLeg.find('\t'));
		currentLeg = currentLeg.substr(currentLeg.find('\t') + 1);
		arrival = stoi(currentLeg);

		//Setting current leg information
		if (planets.find(startPlanet) == planets.end()) {
			departure_planet = new Planet(startPlanet);
			planets[startPlanet] = departure_planet;
		}
		else {
			departure_planet = planets[startPlanet];
		}
		if (planets.find(destPlanet) == planets.end()) {
			destination_planet = new Planet(destPlanet);
			planets[destPlanet] = destination_planet;
		}
		else {
			destination_planet = planets[destPlanet];
		}
		departure_time = dept;
		arrival_time = arrival;
		if (ships.find(ship) == ships.end()) {
			ship_id = ships[ship] = galaxy->fleet.add(ship);
		}

		//Validating current leg. 
		if (validate()) {
			Leg current(ships[ship], dept, arrival);
			if (edges[planets[startPlanet]].find(planets[destPlanet]) == edges[planets[startPlanet]].end()) {
				edges[departure_planet][destination_planet] = new Edge(destination_planet);
				edges[departure_planet][destination_planet]->add(current);
				planets[startPlanet]->add(edges[departure_planet][destination_planet]);
			}
			else {
				edges[departure_planet][destination_planet]->add(current);
			}
			//Set previous leg information for validation. 
			previous_ship_id = ship_id;
			previous_destination_planet = destination_planet;
			previous_arrival_time = arrival_time;
		}
		else { //Set of information that caused the error 
			cerr << "Invalid input. Current Leg displayed below is not well-formed!" << endl; 
			cerr << galaxy->fleet.name(ship_id) << " " << departure_planet->name << " " << destination_planet->name << " " << departure_time << " " << arrival_time << endl;
			exit(EXIT_FAILURE);
		}
	}
	route.close();
	for (auto const &plan : planets) {
		auto const &val = plan.second;
		galaxy->add(val);
	}
}
//**********************************************END OF READER CLASS**********************************************//


//**********************************************START OF GALAXY CLASS**********************************************//

/*
Precondition: None
Postcondition: None
Checks if any planets are unreachable by testing if their best_leg's arrival time is 
MAX_TIME or not. Will exit program if so (Graph is not well formed). 
*/
void Galaxy::checkAllPlanets()
{
	for (unsigned int i = 0; i < planets.size(); i++) {
		if (planets[i]->arrival_time() == MAX_TIME) {
			cerr << "PLANET: " << planets[i]->name << ", IS UNREACHABLE!" << endl; 
			exit(EXIT_FAILURE);
		}
	}
}

/*
Precondition: None
Postcondition: None
Runs Dijkstra's algorithm on each planet utilizing a priority queue. 
Will push all elements into the queue, and get the furthest planet
away from this home planet through dijkstra's algorithm.
*/
void Galaxy::search()
{
	Planet* furthest; //Furthest planet from the home planet. 
	Itinerary* schedule;
	PriorityQueue<Planet, int(*)(Planet*, Planet*)> queue(Planet::compare);
	for (unsigned int i = 0; i < planets.size(); i++) {
		//Pushes all planets back into the priority queue
		for (unsigned int j = 0; j < planets.size(); j++) {
			queue.push_back(planets[j]);
		}
		furthest = planets[i]->search(queue); 
		schedule = planets[i]->make_itinerary(furthest);

		//This for-loop will print out all of the itineraries possible. 
		//Commented out but was ran once pre-submission just as an additional file to see and compare with. 
		//May rerun. Intentionally left out in the final submission to ensure better run-time. 
		/*for (unsigned int j = 0; j < planets.size(); j++) {
			if (j != i) {
				planets[i]->outputAllRoutes(planets[j], fleet);
			}
		}*/

		//planets[i]->dumpPredecessors();
		schedule->print(this, fleet); 
		checkAllPlanets(); //Checks if all planets are reachable. 
		this->reset();
		delete schedule;
	}
}

/*
Precondition: None
Postcondition: None
Dumps out the planets.
*/
void Galaxy::dump()
{
	for (unsigned int i = 0; i < planets.size(); i++) {
		planets[i]->dump(this);
	}
}
//**********************************************END OF GALAXY CLASS**********************************************//

//**********************************************START OF PLANET CLASS**********************************************//

/*
Precondition: None
Postcondition: Returns the furthest planet 
Part of dijkstra's algorithm from Galaxy::search() 
Will get the furthest planet (Last planet in the queue) and return that. 
*/
Planet * Planet::search(PriorityQueue<Planet, int(*)(Planet*, Planet*)>& queue)
{
	best_leg = Leg(-1, 0, 0); //Home planet
	Planet* furthest = this; //Unitialized pointer error otherwise. Needs some form of memory to hold onto. 
	Planet* current;
	queue.reduce(this); //Sift the home planet to top of queue. 
	while (!queue.empty()) {
		current = queue.pop();
		if (queue.empty()) { //Last element in the queue is the farthest planet no matter what due to Dijkstra's algorithm 
			furthest = current; 
			break;
		}
		current->relax_neighbors(queue);
	}
	return furthest;
}

/*
Precondition: None
Postcondition: None
Will dump out the predecessors of the given planet to show the path. 
Does not show times but the general schedule. 
*/
void Planet::dumpPredecessors()
{
	Planet* pred = this; 
	while (pred != nullptr) {
		cerr << pred->name << endl; 
		pred = pred->predecessor;
	}
	cerr << endl; 
}


/*
Precondition: None
Postcondition: None
Helper method that printed out all the itineraries of every single planet. 
Cannot be run during the program. Requires a uncomment or explicit call
somewhere inside the galaxy class methods.
*/
void Planet::outputAllRoutes(Planet * destination, Fleet& fleet)
{
	Itinerary* schedule = new Itinerary(this); 
	while (destination) {
		schedule->destinations.push_back(destination);
		schedule->legs.push_back(destination->best_leg);
		destination = destination->predecessor;
	}
	schedule->printToFile(fleet);
	delete schedule;
}

/*
Precondition: None
Postcondition: Returns an itinerary
Will create the itinerary based on the destination planet passed through and return it. 
Does this by creating a parallel sequence of planets and legs.
*/
Itinerary * Planet::make_itinerary(Planet * destination)
{
	Itinerary* schedule = new Itinerary(this); 
	while(destination){
		schedule->destinations.push_back(destination); 
		schedule->legs.push_back(destination->best_leg); 
		destination = destination->predecessor; 
	}
	return schedule;
}

/*
Precondition: None
Postcondition: None
Checks all neighbors to the given planet and does comparisons on the legs within the edge with the 
best_leg of the destination planet. 
*/
void Planet::relax_neighbors(PriorityQueue<Planet, int(*)(Planet*, Planet*)>& queue)
{
	Time nextMin = best_leg.arrival_time + TRANSFER_TIME; //Arrival Time + 4 hours. 
	Planet* destPlanet;
	for (unsigned int i = 0; i < edges.size(); i++) {
		edges[i]->sort(); //Will get the legs in ascending order. 
		destPlanet = edges[i]->destination;
		Leg bestLeg;
		Leg current;
		for (unsigned int j = 0; j < edges[i]->departures.size(); j++) {
			current = edges[i]->departures[j];
			if (current.departure_time >= nextMin) { //The first leg that follows this boolean expression is the lowest time for the leg. 
				bestLeg = current;
				break;  
			}
		}
		if (Leg::less_than(bestLeg, destPlanet->best_leg)) { //Compares the best leg within the edge to the best leg of the planet. Replaces if necessary. 
			destPlanet->predecessor = this; 
			destPlanet->best_leg = bestLeg; 
			queue.reduce(destPlanet); 
		}
	}
}

/*
Precondition: None
Postcondition: None
Takes in a galaxy pointer to have access to the fleet names. 
Will print out the home planet and all edges from it along with the ships. 
*/
void Planet::dump(Galaxy * galaxy)
{
	cerr << "--------------------------------------------------------------------------" << endl;
	cerr << "HOME PLANET: " << name << endl << endl;
	for (unsigned int j = 0; j < edges.size(); j++) {
		edges[j]->dump(galaxy);
	}
}
//**********************************************END OF PLANET CLASS**********************************************//


//**********************************************START OF EDGE CLASS**********************************************//
/*
http://www.cplusplus.com/reference/algorithm/sort/ <--Credits to
Takes Iterators begin() and end() of a vector and sorts the elements inside based 
a boolean function passed in as a third argument. Algorithm is part of the Algorithm library
and is intended to sort the elements of a vector in ascending order. 
*/
void Edge::sort()
{
	std::sort(departures.begin(), departures.end(), Leg::less_than);
}

/*
Precondition: None
Postcondition: None
See Planet::dump(Galaxy * galaxy); 
*/
void Edge::dump(Galaxy * galaxy)
{
	cerr << "-->DESTINATION PLANET: " << destination->name << endl;
	for (unsigned int i = 0; i < departures.size(); i++) {
		cerr << "SHIP: " << galaxy->fleet.name(departures[i].id) << ", " << departures[i].departure_time << " " << departures[i].arrival_time << endl;
	}
	cerr << endl;
}
//**********************************************END OF EDGE CLASS**********************************************//



//**********************************************START OF ITINERARY CLASS**********************************************//

/*
Precondition: None
Postcondition: None
Prints out the itinerary for the given itinerary. 
Will print out the itinerary for a given galaxy planet. 
Will also write out to a file, the highest arrival time of a given pair of planets.
*/
void Itinerary::print(Galaxy* galaxy, Fleet & fleet, std::ostream & out)
{
	//Prints the longest journey of a given planet to a file called 'sampleRoute.txt' 
	if (destinations[0]->arrival_time() > galaxy->highestTime) {
		galaxy->highestTime = destinations[0]->arrival_time();
		ofstream outFile("sampleRoute.txt");
		for (int i = destinations.size() - 1; i > 0; i--) {
			outFile << fleet.name(legs[i - 1].id) << '\t' << destinations[i]->name << '\t' << legs[i - 1].departure_time << '\t' << destinations[i - 1]->name << '\t' << legs[i - 1].arrival_time << endl;
		}
		outFile.close();
	}

	//Prints the longest-shortest path of each pairs of planets in the galaxy
	for (int i = destinations.size() - 1; i > 0; i--) {
		out << fleet.name(legs[i - 1].id) << '\t' << destinations[i]->name << '\t' << legs[i - 1].departure_time << '\t' << destinations[i - 1]->name << '\t' << legs[i - 1].arrival_time << endl;
	}
	out << endl;
}

/*
Precondition: None
Postcondition: None
Prints out the itinerary in the same format as Itinerary::print(Galaxy* galaxy, Fleet & fleet, std::ostream & out);
*/
void Itinerary::printToFile(Fleet& fleet)
{
	ofstream outFile("AllItineraries.txt", ios::out | ios::app);
	for (unsigned int i = destinations.size() - 1; i > 0; i--) {
		outFile << fleet.name(legs[i - 1].id) << '\t' << destinations[i]->name << '\t' << legs[i - 1].departure_time << '\t' << destinations[i - 1]->name << '\t' << legs[i - 1].arrival_time << endl;
	}
	outFile << endl; 

	outFile.close(); 
}

//**********************************************END OF ITINERARY CLASS**********************************************//
