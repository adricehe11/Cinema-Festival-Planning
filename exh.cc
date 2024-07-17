// Authors: Sergio Cárdenas & Adrián Cerezuela

/* Cerca exhaustiva. Cada vegada que obtingui una solució millor, l’ha de
sobreescriure al fitxer de sortida, és a dir, volem que si avortem el programa,
dins el fitxer de sortida hi hagi la millor solució trobada fins al moment. */

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std; 

using Day = vector<int>;

/* For each film, we will need to know its title, if it is already planned to
be projected, and its restrictions with the other ones. */
struct Film {
  string title;
  bool projected = false;
  vector<bool> restrictions;
};

int f, l, c, min_d, best_case;
double start_time, end_time;

string output_file;
ifstream in;
ofstream out;

double now() { return clock() / double(CLOCKS_PER_SEC); }

/* Reads the film titles, adds them to their corresponding films structure while
initializing the restrictions vectors and returns the film structure vector. */
vector<Film> read_films() {
  vector<Film> films(f);
  vector<bool> ini_restrictions(f, false);
  for (int i = 0; i < f; ++i) {
    in >> films[i].title;
    films[i].restrictions = ini_restrictions;
  }
  return films;
}

/* Reads a pair of films that cannot be projected on the same day and declares
that restriction on their restrictions vectors. */
void read_restrictions(vector<Film> &films) {
  for (int i = 0; i < l; ++i) {
    string film1, film2;
    in >> film1 >> film2;

    int f1 = -1, f2 = -1;
    int j = 0;
    while (f1 == -1 or f2 == -1) {
      if (film1 == films[j].title)
        f1 = j;
      if (film2 == films[j].title)
        f2 = j;
      ++j;
    }

    films[f1].restrictions[f2] = true;
    films[f2].restrictions[f1] = true;
  }
}

// Reads the cinemas' names and returns the corresponding string vector.
vector<string> read_cinemas() {
  vector<string> cinemas(c);
  for (int i = 0; i < c; ++i)
    in >> cinemas[i];
  return cinemas;
}

/* Returns the possible minimum number of days where we can put f films in c
cinemas without taking restrictions into account. */
int min_days() {
  if (f % c != 0)
    return int(f / c) + 1;
  return f / c;
}

// Returns the length of the longest film title
int max_length(const vector<Film> &films) {
  int max = 0;
  for (int i = 0; i < f; ++i) {
    if (films[i].title.size() > max)
      max = films[i].title.size();
  }
  return max;
}

/* Given a matrix containing the best planning and the vectors
containing the films and cinemas' names, writes the festival planning based
on the output format. */
void write(const vector<Day> &plan, const vector<Film> &films,
           const vector<string> &cinemas) {

  ofstream out(output_file);
  out.setf(ios::fixed);
  out.precision(1);
  // sets the output up to a decimal

  end_time = now();
  double time = end_time - start_time;

  out << time << endl;
  out << min_d << endl;
  int max_spaces = max_length(films) + 3;
  /* as long as it is aligned and satisfies the output format, we consider 
  the max spaces to be these ones. */

  for (int i = 0; i < plan.size(); ++i) {
    for (int j = 0; j < plan[i].size(); ++j) {
      int spaces = max_spaces - films[plan[i][j]].title.size();
      out << films[plan[i][j]].title << string(spaces, ' ') << i + 1 << "    "
          << cinemas[j] << endl;
    }
  }
  out.close();
}

// Returns the worst plan matrix, that is the one with a film per day.
vector<Day> generate_worst_plan() {
  vector<Day> worst_plan(min_d);
  for (int i = 0; i < min_d; ++i)
    worst_plan[i].push_back(i);
  return worst_plan;
}

/* Boolean function that returns true if we have any restriction between the
current film and the ones already planned on an specific day. */
bool restricted(const Film current_film, Day day_plan) {
  for (int j = 0; j < day_plan.size(); ++j) {
    if (current_film.restrictions[day_plan[j]])
      return true;
  }
  return false;
}

/* Function that generates plans of d days without restrictions using
exhaustive search and writes the best_plan with the minimum number of days. */
void exhaustive_search_planning(vector<Film> &films, vector<string> &cinemas,
                                vector<Day> &current_plan, vector<Day> &best_plan,
                                int k, int d) {
  if (k == f) {
    min_d = d;
    best_plan = current_plan;
    write(best_plan, films, cinemas);
    /* Plans with d > min_d days will have more days than our current best_plan,
    and if best_case = min_d we already have a plan with the possible minimum
    number of days, so there is no need to continue generating plans */
  } else if (d - 1 < min_d and best_case < min_d) {
    for (int i = 0; i < f; ++i) {
      if (not films[i].projected and not restricted(films[i], current_plan[d - 1])) {
        current_plan[d - 1].push_back(i);
        films[i].projected = true;
        if (current_plan[d - 1].size() == c and k + 1 < f)
          exhaustive_search_planning(films, cinemas, current_plan, best_plan, k + 1, d + 1);
        else
          exhaustive_search_planning(films, cinemas, current_plan, best_plan, k + 1, d);
        current_plan[d - 1].pop_back();
        films[i].projected = false;
      }
    }
  }
}

int main(int argc, char **argv) {
  string input_file = argv[1];
  output_file = argv[2];

  in.open(input_file);

  start_time = now();

  in >> f;
  vector<Film> films = read_films();

  in >> l;
  read_restrictions(films);

  in >> c;
  vector<string> cinemas = read_cinemas();

  best_case = min_days();
  min_d = f;
  /* min_d keeps the minimum number of days of the best_plan. It is initialized
  with the worst possible plan, where only a film per day can be projected */

  vector<Day> best_plan = generate_worst_plan();
  write(best_plan, films, cinemas);

  vector<Day> current_plan(min_d);
  exhaustive_search_planning(films, cinemas, current_plan, best_plan, 0, 1);
}
