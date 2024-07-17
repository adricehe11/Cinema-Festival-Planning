// Authors: Sergio Cárdenas & Adrián Cerezuela

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

using Day = vector<int>;

// We will use this structure to order the films considering their restrictions.
struct Film_info {
  int idx;
  vector<bool> restrictions;
};

// We will use this structure to store the solutions and their info.
struct Solution {
  vector<Day> plan;
  int cost;
  int days;
  int restrictions;
};

int f, l, c;
double start_time, end_time;

string output_file;
ifstream in;
ofstream out;

double now() { return clock() / double(CLOCKS_PER_SEC); }

/* Reads the film name, adds it to the films_titles vector and films structure,
including its index in it. */
void read_films(vector<Film_info> &films_info, vector<string> &films_titles) {
  vector<bool> ini_restrictions(f, false);
  for (int i = 0; i < f; ++i) {
    in >> films_titles[i];
    films_info[i].idx = i;
    films_info[i].restrictions = ini_restrictions;
  }
}

/* Reads a pair of films that cannot be projected on the same day and declares
that restriction on their restrictions vectors. */
void read_restrictions(vector<Film_info> &films_info,
                const vector<string> &films_titles) {
  for (int i = 0; i < l; ++i) {
    string film1, film2;
    in >> film1 >> film2;

    int f1 = -1, f2 = -1;
    int j = 0;
    while (f1 == -1 or f2 == -1) {
      if (film1 == films_titles[j])
        f1 = j;
      if (film2 == films_titles[j])
        f2 = j;
      ++j;
    }

    films_info[f1].restrictions[f2] = true;
    films_info[f2].restrictions[f1] = true;
  }
}

// Reads the cinemas' names and returns the corresponding string vector.
void read_cinemas(vector<string> &cinemas) {
  for (int i = 0; i < c; ++i)
    in >> cinemas[i];
}

/* Boolean function that checks if a film can be planned on an specific day, based on the ones
already planned on that day. */
bool restricted(Day day_plan, Film_info f_i) {
  for (int j = 0; j < day_plan.size(); ++j) {
    if (f_i.restrictions[day_plan[j]])
      return true;
  }
  return false;
}

// Returns the length of the longest film title.
int max_length(const vector<string> &films) {
  int max = 0;
  for (string s : films) {
    if (s.size() > max)
      max = s.size();
  }
  return max;
}

/* Given a matrix with the best planning and the vectors
containing the films' titles and cinemas, prints the festival planning based
on the output format. */
void write(const vector<Day> &plan, const vector<string> &films_titles,
           const vector<string> &cinemas, int d) {
  ofstream out(output_file);
  out.setf(ios::fixed);
  out.precision(1);
  // sets the output up to a decimal

  end_time = now();
  double time = end_time - start_time;

  out << time << endl;
  out << d << endl;
  int max_spaces = max_length(films_titles) + 3;
  /* we consider the max number of spaces beetween the titles and the
  projection days to be the length of the longest film plus 3 spaces */

  for (int i = 0; i < plan.size(); ++i) {
    for (int j = 0; j < plan[i].size(); ++j) {
      int spaces = max_spaces - films_titles[plan[i][j]].size();
      out << films_titles[plan[i][j]] << string(spaces, ' ') << i + 1 << "    "
          << cinemas[j] << endl;
    }
  }
}

/* Given the films' informations, it will make the plan
in a number d of days using a greedy algorithm. */
void generate_planning(const vector<Film_info> &films_info, vector<Day> &plan,
                 int &d) {
  // The most restricted film is put on the first day of the plan
  plan[0].push_back(films_info[0].idx);

  for (int i = 1; i < f; ++i) {
    /* the next most restricted film is inserted on the plan, checking if it is
    possible to do it on the same day or if we will need to go to the next one */

    if (plan[d].size() == c or restricted(plan[d], films_info[i])) {
      d += 1;
      plan[d].push_back(films_info[i].idx);
    } else
      plan[d].push_back(films_info[i].idx);
  }
}

/* Completes the parameters of a certain solution, based on a given planning. */
Solution fill_solution(const vector<Day> &plan, int d) {
  Solution current;
  current.plan = plan;
  current.days = d + 1;
  current.restrictions = 0;
  current.cost = current.days + 1000 * current.restrictions;
  return current;
}

/* Given a film, a certain planning day and a vector of films' informations,
  returns the number of restrictions the film has on that determined day. */
int count_restrictions(int film, const Day &day_films,
                       const vector<Film_info> &films_info) {
  int count = 0;
  for (int i = 0; i < day_films.size(); ++i) {
    if (films_info[film].restrictions[day_films[i]]) {
      ++count;
    }
  }
  return count;
}

/* Moves a film from a certain day to another and returns the difference between 
the restrictions with and without moving it. */
int neighbour_restrictions(Solution &neighbour,
                           const vector<Film_info> &films_info) {
  // two days are randomly chosen: one to remove a film and another to place it
  int old_day = -1;
  int new_day = -1;

  // these two random numbers have to be different
  while (old_day == new_day) {
    old_day = rand() % neighbour.days;
    while (neighbour.plan[old_day].size() == 0)
      old_day = rand() % neighbour.days;
    new_day = rand() % (neighbour.days + 1);
    while (new_day < neighbour.days and neighbour.plan[new_day].size() >= c)
      new_day = rand() % (neighbour.days + 1);
  }

  int old_restrictions = count_restrictions(
      neighbour.plan[old_day].back(), neighbour.plan[old_day], films_info);

  // the film is moved from the old day to the new day
  if (new_day == neighbour.days) {
    ++neighbour.days;
    neighbour.plan.push_back({neighbour.plan[old_day].back()});
  } else {
    neighbour.plan[new_day].push_back(neighbour.plan[old_day].back());
  }
  neighbour.plan[old_day].pop_back();

  int new_restrictions = count_restrictions(
      neighbour.plan[new_day].back(), neighbour.plan[new_day], films_info);

  return new_restrictions - old_restrictions;
}

// Given a current solution, returns a certain neighbour solution.
Solution find_neighbour(const Solution &current,
                        const vector<Film_info> &films_info) {
  Solution neighbour;
  neighbour.plan = current.plan;
  neighbour.days = neighbour.plan.size();
  neighbour.restrictions =
      current.restrictions + neighbour_restrictions(neighbour, films_info);
  neighbour.cost = neighbour.days + 1000 * neighbour.restrictions;
  return neighbour;
}

/* Returns a certain probability computed with the Boltzmann distribution, 
based on the costs of the current solution, a certain neighbour 
and the temperature T. */
double probability(double T, const Solution &current,
                   const Solution &neighbour) {
  return exp((current.cost - neighbour.cost) / T);
}

// Generates a random number beetween 0 and 1.
double generate_random() {
  random_device rd;
  default_random_engine eng(rd());
  uniform_real_distribution<double> distr(0, 1);
  return distr(eng);
}

/* With probability p, takes the worst solution between the current 
one and a certain neighbour. */
void update(double p, Solution &current, const Solution &neighbour) {
  double r = generate_random();
  if (not(r > p))
    current = neighbour;
}

/* Applies a simulated annealing algorithm, where temperature is reduced at 
each iteration and  moves resulting in solutions of worse quality than the 
current one are allowed in order to escape from local optima. */
void simulated_annealing(const vector<string> &films_titles,
                         const vector<Film_info> &films_info,
                         const vector<string> &cinemas, const vector<Day> &plan,
                         Solution &optimal, int &d, double T) {
  Solution current = fill_solution(plan, d);
  if (current.days < optimal.days)
    optimal = current;
  write(optimal.plan, films_titles, cinemas, optimal.days);

  int k = 0;
  while (k < 10000) {
    Solution neighbour = find_neighbour(current, films_info);
    if (neighbour.cost < current.cost) {
      current = neighbour;
      if (current.restrictions == 0 and current.days < optimal.days) {
        optimal = current;
        write(optimal.plan, films_titles, cinemas, optimal.days);
        k = -1;
      }
    } else {
      double p = probability(T, current, neighbour);
      update(p, current, neighbour);
    }
    T *= 0.99;
    ++k;
  }
}

/* Given a planning, clears it out, so that a new one can be generated 
without declaring a new matrix. */
void clear_out_plan(vector<Day> &plan) {
  for (int d = 0; d < plan.size(); ++d) {
    while (not plan[d].empty())
      plan[d].pop_back();
  }
}

int main(int argc, char **argv) {
  string input_file = argv[1];
  output_file = argv[2];
  in.open(input_file);

  start_time = now();

  in >> f;
  vector<Film_info> films_info(f);
  vector<string> films_titles(f);
  read_films(films_info, films_titles);

  in >> l;
  read_restrictions(films_info, films_titles);

  in >> c;
  vector<string> cinemas(c);
  read_cinemas(cinemas);


  /* The films are randomly sorted before generating an initial greedy solution, 
  so that a different one is generated every iteration before applying the metaheuristics. */
  random_shuffle(films_info.begin(), films_info.end());

  int d = 0;
  vector<Day> current_plan(f);
  generate_planning(films_info, current_plan, d);
  Solution optimal = fill_solution(current_plan, d);

  while (true) {
    simulated_annealing(films_titles, films_info, cinemas, current_plan, optimal, d,
                        0.99);

    random_shuffle(films_info.begin(), films_info.end());

    d = 0;
    clear_out_plan(current_plan);
    generate_planning(films_info, current_plan, d);
  }
}
