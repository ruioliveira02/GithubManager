# Github Manager

This project was developed for the IT Laboratories III class in the year 2021/2022 by [Rui Oliveira](https://github.com/ruioliveira02), [Tiago Bacelar](https://github.com/tiago-bacelar), and [Luís Pereira](https://github.com/lumafepe), and was evaluated with a XX/20.

The goal of the project was to create a management system for a possibly larger than RAM dataset containing information about Github users, commits and repositories, without using a database, in the C programming language.

## Project Overview

### Dataset Management

The dataset was handled by using intermediate auxiliary files which would index the original files - which were ```.csv``` - by some property of the type: id of a user, date of a commit .etc. This means to search for a given user/repo/commit by a certain parameter, one would look at the corresponding index file and binary search over it, without loading more than one line into memory at once. Binary search is possible as the lines of an indexing file have the same number of bytes, making it trivial to compute the starting byte of the  ```n```-the line.

Another optimization is the fact that a line is read lazily, meaning that only the requested information (like date of a commit, number of commits of a repo) will be read, reducing the time spent reading from disk. 

However, the biggest accomplishment in this project is the constant memory usage - around 1GB - independent of the size of the dataset. This is because a caching system was implemented, meaning whenever a line was read, it would be stored in the cache (plus some other lines consecutive to it), which will eliminate the need to look again for that information in a file.

A more detailed analysis of the performance is available in the provided report.

### GUI

The user interface was developed with the help of the [ncursesw](https://pt.wikipedia.org/wiki/Ncurses) library. The GUI allows for a user to read the dataset, search for a given string and execute some queries regarding it. The GUI is separated into pages, each one corresponding to one task. The page is rendering using a set of components, which allows for an abstract representation of what is actually rendered to the screen, as well as reutilizing them in different pages.

### Testing

A set of tests were created to access the correctness of the program, which are provided in the ```tests/correctness``` folder. There were also developed tests for the performance of the application. However, because of their large size (several GB), these are not provided in this repository.

## Screenshots

![image](https://user-images.githubusercontent.com/70754369/153381510-5c823b78-3442-4d2e-91b8-b19c33629394.png)

![image](https://user-images.githubusercontent.com/70754369/153381682-9488ea1f-f1d4-4360-9cbe-06dfece7e91a.png)

![image](https://user-images.githubusercontent.com/70754369/153381855-2470c79b-61e9-4a15-a241-058cbbf8eea4.png)

![image](https://user-images.githubusercontent.com/70754369/153382046-ae3e4bab-4f24-479e-b296-350280f5d48a.png)

![image](https://user-images.githubusercontent.com/70754369/153382202-2dcf2cd7-a5c6-44bd-9cd5-f6f29d5eaf3a.png)


## Running the project

Running the project is as simple as running ```make``` in the root directory. To run the program, call ```./github-manager```. To run the tests, compile using ```make test``` and execute with ```./test default```.

Please note the project is ***not compatible*** with Windows.

## Improving the project

There are several ways in which the project could be improved. The addition of a garbage collector to allow some ugly ```free``` to be removed (especially in the GUI), transforming the GUI into a server/client architecture using pipes, making the GUI even more responsive (something like [Bootstrap](https://getbootstrap.com/) grids), a more advanced search routine .etc.

Overall, the group is very happy with the outcome of this project, which was evaluated with a XX score of 20.
