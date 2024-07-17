// Authors: Sergio Cárdenas & Adrián Cerezuela

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

using Day = vector<int>;

/* For each film, we will need to know its numeric identifier (index), its
number of restrictions, and its restrictions with the other ones. */
struct Film_info {
  int idx;
  int num_restrictions = 0;
  vector<bool> restrictions;
};

int f, l, c;
double start_time, end_time;

string output_file;
ifstream in;
ofstream out;

double now() { return clock() / double(CLOCKS_PER_SEC); }

/* Reads the films' titles, adds them to the films_titles vector and initializes
its index and restriction vector in the structure. */
void read_films(vector<Film_info> &films_info, vector<string> &films_titles) {
  vector<bool> ini_restrictions(f, false);
  for (int i = 0; i < f; ++i) {
    in >> films_titles[i];
    films_info[i].idx = i;
    films_info[i].restrictions = ini_restrictions;
  }
}

/* Reads a pair of films that cannot be projected on the same day, declares that
restriction on their restriction vectors and increases the count of restrictions
of that film. */
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
    ++films_info[f1].num_restrictions;
    ++films_info[f2].num_restrictions;
  }
}

// Reads the cinemas' names and returns the corresponding string vector.
vector<string> read_cinemas() {
  vector<string> cinemas(c);
  for (int i = 0; i < c; ++i)
    in >> cinemas[i];
  return cinemas;
}

/* If two films have different number of restrictions, returns the most
restricted. Otherwise, returns the one with the lowest index. */
bool film_sorter(Film_info const &f1, Film_info const &f2) {
  if (f1.num_restrictions != f2.num_restrictions)
    return f1.num_restrictions > f2.num_restrictions;
  return f1.idx < f2.idx;
}

// Returns the length of the longest film title.
int max_length(const vector<string> &films_titles) {
  int max = 0;
  for (string s : films_titles) {
    if (s.size() > max)
      max = s.size();
  }
  return max;
}

/* Given a matrix containing the plan and the vectors containing the films
titles and cinemas' names, writes the festival planning based on the output
format. */
void write(const vector<Day> &plan, const vector<string> &films_titles,
           const vector<string> &cinemas, int d) {
  ofstream out(output_file);
  out.setf(ios::fixed);
  out.precision(1);
  // sets the output up to a decimal

  end_time = now();
  double time = end_time - start_time;

  out << time << endl;
  out << d + 1 << endl;
  int max_spaces = max_length(films_titles) + 3;
  /* As long as it is aligned and satisfies the output format, we consider
  the max spaces to be these ones. */

  for (int i = 0; i < plan.size(); ++i) {
    for (int j = 0; j < plan[i].size(); ++j) {
      int spaces = max_spaces - films_titles[plan[i][j]].size();
      out << films_titles[plan[i][j]] << string(spaces, ' ') << i + 1 << "    "
          << cinemas[j] << endl;
    }
  }
  out.close();
}

/* Boolean function that returns true if we have any restriction between the
current film and the ones already planned on an specific day. */
bool restricted(const Film_info current_film, Day day_plan) {
  for (int j = 0; j < day_plan.size(); ++j) {
    if (current_film.restrictions[day_plan[j]])
      return true;
  }
  return false;
}

// Generates a plan of d days without restrictions using a greedy algorithm.
void greedy_planning(vector<Film_info> &films_info,
                     const vector<string> &films_titles,
                     const vector<string> &cinemas, vector<Day> &plan, int d) {
  // The most restricted film index is put on the first day of the plan;
  plan[0].push_back(films_info[0].idx);

  for (int i = 1; i < f; ++i) {
    /* For all films, the following most restricted film index is picked, which
    is placed in the plan, checking if it is possible to do it on the same day
    or if we will need to go to the next one. */
    if (plan[d].size() == c or restricted(films_info[i], plan[d])) {
      d += 1;
      plan[d].push_back(films_info[i].idx);
    } else
      plan[d].push_back(films_info[i].idx);
  }
  write(plan, films_titles, cinemas, d);
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
  vector<string> cinemas = read_cinemas();

  // Sorts the film_info by number of restrictions in descending order;
  sort(films_info.begin(), films_info.end(), film_sorter);

  int d = 0;
  vector<Day> plan(f);
  greedy_planning(films_info, films_titles, cinemas, plan, d);
}
