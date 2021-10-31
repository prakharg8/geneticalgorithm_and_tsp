#include "SFML\Graphics.hpp"
#include "SFML\System.hpp"
#include "SFML\Window.hpp"
#include "SFML\Network.hpp"
#include "SFML\Audio.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>

using namespace sf;
using namespace std;

double distance(Vector2f a, Vector2f b) {
    return (double)pow(abs(a.x - b.x), 2) + (double)pow(abs(a.y - b.y), 2);
}

//calculates fitness of a path
double calcFitness(VertexArray& cities, vector<int> &order, vector<vector<double>>& distances) {
    double sum = 0;
    int size = (int)order.size();
    for (int i = 0; i < size-1; i++) {
        sum += distances[order[i]][order[i+1]];
    }
    sum = 1.0 / (sum + 1.0);
    //cout << 1/ (sum + 1.0) << endl;
    return pow(sum, 30);
    //return 1 / (sum + 1.0);
    //return -1.0 / (1.0 - expl(sum));
}
//calculates total distance
double calcDistance(VertexArray& cities, vector<int> &order, vector<vector<double>>& distances) {
    double sum = 0;
    int size = (int)order.size();
    for (int i = 0; i < size-1; i++) {
        sum += distances[order[i]][order[i+1]];
    }
    return sum;
}
//normalise the fitnesses
void normaliseFitness(vector<double>& fit) {
    double sum = 0;
    int size = (int)fit.size();
    for (int i = 0; i < size; i++) {
        //cout << fit[i] /*<< " ";
        //cout << endl;*/
        sum += fit[i];
    }
    for (auto& it : fit)it /= sum;
}
//pool selection for the next generation
vector<int> pickOne(vector<vector<int>>& list, vector<double>& probabilities) {
    double random = ((double)rand() + 1.0) / (double)(RAND_MAX);
    int index = 0;
    while (random > 0) {
        random = random - probabilities[index];
        //cout << index << endl;
        index++;
    }
    index--;
    return list[index];
}
//mutation for next generation
void mutate(vector<int>& order, double mutationrate, int type=1) {
    for (int i = 0, s = (int)order.size(); i < s; i++) {
		double random = ((double)rand() + 1.0) / (double)(RAND_MAX);
        if (random < mutationrate) {
			int a = rand() % (int)order.size();
			int b = rand() % (int)order.size();
			if(type == 1) swap(order[a], order[b]);
            else if(type == 2) swap(order[a], order[(a + 1) % s]);
        }
    }
}
//crossover function for the next generation
vector<int> crossOver(vector<int>& orderA, vector<int>& orderB) {
    int size = (int)orderA.size();
    int begin = rand() % size;
    int end = begin + (rand() % (size - begin));
    vector<int> order;
    copy(orderA.begin() + begin + 1, orderA.begin() + end + 1, back_inserter(order));
    for (int i = 0; i < size; i++) {
        bool f = true;
        for (int j = 0, s = (int)order.size(); j < s; j++) {
            if (order[j] == orderB[i]) {
                f = false;
                break;
            }
        }
        if (f) {
            order.push_back(orderB[i]);
        }
    }
    return order;
}
//create next generation
void nextGen(vector<vector<int>>& population, vector<double>& fitnesses, double mutationrate, int type=1) {
    vector<vector<int>> newpop;
    for (int i = 0, s = (int)population.size(); i < s; i++) {
        vector<int> orderA = pickOne(population, fitnesses);
        vector<int> orderB = pickOne(population, fitnesses);
        vector<int> order = crossOver(orderA, orderB);
        mutate(order, mutationrate, type);
        newpop.push_back(order);
    }
    population = newpop;
}




int main()
{
    srand((int)time(0));
    int height = 700, width = 1000;
    RenderWindow window(VideoMode(3 * width / 2, height), "genetic algorithm vs nearest neighbour");
    //RenderWindow window2(VideoMode(width, height), "nearest neighbour algorithm");
    //window.setFramerateLimit(60);

    //creating random cities
    int totalCities = 50;
    VertexArray cities(LinesStrip);
    for (int i = 0; i < totalCities; i++) {
        cities.append(Vertex(Vector2f((double)(rand() % ((width - 10) / 2)), (double)(rand() % ((height - 10) / 2)))));
    }

    Transform down, side, downside, side2;
    downside.translate((width - 5) / 2, (height - 5) / 2);
    side2.translate((width - 5), 5);
    side.translate((width - 5) / 2, 5);
    down.translate(5, (height - 5) / 2);

    //creating random population
    vector<int> order(totalCities);
    for (int i = 0; i < totalCities; i++)order[i] = i;
    int popSize = 2000;
    double mutationrate = 0.05;
    double nn_mutationrate = 0.05;
    random_device rg;
	auto rng = default_random_engine{rg()};
    vector<vector<int>> population(popSize, order);
    for (auto& it : population) {
        shuffle(begin(it), end(it), rng);
    }

    //finding distances between all the cities
    vector<vector<double>> distances(totalCities, vector<double>(totalCities, 0));
    for (int i = 0; i < totalCities; i++) {
        for (int j = 0; j < totalCities; j++) {
            distances[i][j] = distance(cities[i].position, cities[j].position);
        }
    }

    //nearest neighbour algorithm
    vector<int> done(totalCities, 0);
    vector<int> nn_order;
    VertexArray path(LineStrip);
    int i = 0;
    path.append(cities[0]);
    nn_order.push_back(0);
    while (path.getVertexCount() < totalCities) {
        int m = -1;
        double mi = numeric_limits<double>::max();
        for (int j = 0; j < totalCities; j++) {
            if (i == j)continue;
            if (done[j] == 1)continue;
            if (distances[i][j] < mi) {
                mi = distances[i][j];
                m = j;
            }
            //cout << m << endl;
        }
        if (m > 0) {
            //cout << m << endl;
            nn_order.push_back(m);
            path.append(cities[m]);
        }
        done[i] = 1;
        i = m;
    }
    int last = i;
    vector<vector<int>> nn_population(popSize, nn_order);
    //for (auto& it : nn_population) {
    //    mutate(it, nn_mutationrate);
    //}

    //loop
    vector<int> bestEver = order;
    vector<int> nn_bestEver = nn_order;
    double bestfitnessEver = calcFitness(cities, order, distances);
    double nn_bestfitnessEver = calcFitness(cities, nn_order, distances);
    int gen = 0, type = 2, nn_type = 2;
    while (window.isOpen())
    {
        cout << gen << endl;
        //if (gen > 1000 && type == 2) type = 1;
        gen++;
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        //finding all the fitnesses
		vector<double> fitnesses;
        vector<double> nn_fitnesses;
		for (auto& it : population) {
			double fit = calcFitness(cities, it, distances);
            if (fit > bestfitnessEver) {
                bestfitnessEver = fit;
                bestEver = it;
            }
			fitnesses.push_back(fit);
		}
		normaliseFitness(fitnesses);  
		for (auto& it : nn_population) {
			double fit = calcFitness(cities, it, distances);
            if (fit > nn_bestfitnessEver) {
                nn_bestfitnessEver = fit;
                nn_bestEver = it;
            }
			nn_fitnesses.push_back(fit);
		}
		normaliseFitness(nn_fitnesses);  

        //create next gen
        nextGen(population, fitnesses, mutationrate, type);
        nextGen(nn_population, nn_fitnesses, nn_mutationrate, 2);

		//finding the best path
		vector<int> best = order;
		VertexArray bestPath(LineStrip);
		VertexArray bestpathEver(LineStrip);
		double bestFitness = calcFitness(cities, order, distances);
		for (int i = 0, s = (int)fitnesses.size(); i < s; i++) {
			if (bestFitness < fitnesses[i]) {
				bestFitness = fitnesses[i];
				best = population[i];
			}
		}
		for (auto i:best) {
			bestPath.append(cities[i]);
		}
		for (auto i:bestEver) {
			bestpathEver.append(cities[i]);
		}
		vector<int> nn_best = nn_order;
		VertexArray nn_bestPath(LineStrip);
		VertexArray nn_bestpathEver(LineStrip);
		double nn_bestFitness = calcFitness(cities, nn_order, distances);
		for (int i = 0, s = (int)nn_fitnesses.size(); i < s; i++) {
			if (nn_bestFitness < nn_fitnesses[i]) {
				nn_bestFitness = nn_fitnesses[i];
				nn_best = nn_population[i];
			}
		}
		for (auto i:nn_best) {
			nn_bestPath.append(cities[i]);
		}
		for (auto i:nn_bestEver) {
			nn_bestpathEver.append(cities[i]);
		}
        
        double nd = calcDistance(cities, bestEver, distances), nnd = calcDistance(cities, nn_bestEver, distances);
        if (nd > nnd) {
            cout << "hybrid" << endl;
        }
        else if (nd == nnd) cout << "equal" << endl;
        else cout << "normal" << endl;
        //cout << nd << " " << nnd << endl;

        //drawing everything
        window.clear();
        window.draw(bestPath, down);
        window.draw(bestpathEver);
        window.draw(nn_bestpathEver, side);
        window.draw(nn_bestPath, downside);
        window.draw(path, side2);
        for (int i = 0; i < totalCities; i++) {
            int radius = 5;
            CircleShape city(radius);
            city.setOrigin(radius, radius);
            city.setFillColor(Color(105, 105, 105));
            city.setPosition(cities[i].position);
            window.draw(city);
            window.draw(city, side);
            window.draw(city, side2);
            window.draw(city, down);
            window.draw(city, downside);
        }
        window.display();
    }
    return 0;
}
