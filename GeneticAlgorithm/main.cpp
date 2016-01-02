#include <iostream> 
#include <vector>
#include <random>
#include <functional>
#include <chrono>
#include <algorithm>
#include <map>
#include <numeric>

using namespace std;

char options[] = { '1','2','3','4','5','6','7','8','9','+','-','*','+','-','*' };

struct Chromosome {
	Chromosome(const vector<int>& indices, double fitness) : indices(indices), fitness(fitness) {}
	vector<int> indices;
	double fitness;
};

void processStack(vector<int>& stack, function<int(int, int)> func) {
	if (stack.size() >= 2) {
		int a = stack.back();
		stack.pop_back();
		int b = stack.back();
		stack.pop_back();
		stack.push_back(func(a, b));
	}
}

int processStack(const vector<int>& indices) {
	vector<int> stack = vector<int>();
	for_each(indices.begin(), indices.end(), [&](auto i) {
		switch (options[i]) {
		case '+':
			processStack(stack, [](int a, int b) { return a + b; });
			break;
		case '-':
			processStack(stack, [](int a, int b) { return a - b; });
			break;
		case '*':
			processStack(stack, [](int a, int b) { return a * b; });
			break;
		default:
			stack.push_back((int)options[i] - 48);
			break;
		}
	});
	return (stack.size() > 1 || stack.size() == 0) ? 0 : stack.back();
}

bool isDigit(int i) {
	return (i > 0) && i < 10;
}

vector<int> postProcessIndices(const vector<int>& indices) {
	vector<int> processedIndices = vector<int>();
	bool seekNum = true;
	for_each(indices.begin(), indices.end(), [&](int i) {
		int num = (int)options[i] - 48;
		if (seekNum && isDigit(num)) {
			processedIndices.push_back(i);
			seekNum = false;
		} else if (!seekNum && !isDigit(num)) {
			processedIndices.push_back(i);
			seekNum = true;
		}
	});
	if (isDigit(processedIndices.back()))
		processedIndices.pop_back();

	return processedIndices;
}

vector<int> genNewIndices(unsigned int amount) {
	vector<int> indices = vector<int>();
	auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	auto dice_rand = bind(uniform_int_distribution<int>(0, (sizeof(options) / sizeof(char)) - 1), mt19937(seed));
	for (unsigned int i = 0; i < amount; i++)
		indices.push_back(dice_rand());
	return postProcessIndices(indices);
}

int randomIndexFromProbabilityWheel(const vector<double>& probabilties) {
	vector<double> cumulativeProbabilties = probabilties;
	for (unsigned int i = 1; i < cumulativeProbabilties.size(); i++) 
		cumulativeProbabilties.at(i) += cumulativeProbabilties.at(i - 1);
	cumulativeProbabilties.insert(cumulativeProbabilties.begin(), 0.0);

	auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	auto dice_rand = bind(uniform_real_distribution<double>(0, 1), mt19937(seed));
	double r = dice_rand();
	for (unsigned int i = 0, j = 1; j < cumulativeProbabilties.size(); i++, j++) 
		if (r >= cumulativeProbabilties.at(i) && r < cumulativeProbabilties.at(j)) return j - 1;
	return 0;
}

vector<double> probabilitiesFromPopulation(const vector<Chromosome>& population) {
	double totalFitnessScore = accumulate(population.begin(), population.end(), 0, [](double fitness, const Chromosome& chromosome) {
		return fitness + chromosome.fitness;
	});

	vector<double> probabilities = vector<double>();
	for_each(population.begin(), population.end(), [&](Chromosome chromosome) {
		probabilities.push_back(chromosome.fitness / totalFitnessScore);
	});
	return probabilities;
}

vector<Chromosome> generatePopulation(int amount, int numGenes, int targetNumber) {
	vector<Chromosome> population = vector<Chromosome>();
	for (int i = 0; i < amount; i++) {
		vector<int> indices = genNewIndices(numGenes);
		int result = processStack(indices);
		double difference = targetNumber - result;
		if (difference == 0)
			difference = 0.99;
		population.push_back(Chromosome(indices, abs(1.0 / (difference))));
	}
	return population;
}

int main(int argc, char* argv[]) {
	vector<Chromosome> population = generatePopulation(10, 20, 42);
	vector<double> chromosomeProbabilities = probabilitiesFromPopulation(population);

	int chosenIndexA = randomIndexFromProbabilityWheel(chromosomeProbabilities);
	int chosenIndexB = randomIndexFromProbabilityWheel(chromosomeProbabilities);
}