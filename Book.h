#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <sys/stat.h> // For setting file permissions (POSIX systems)
using namespace std;

// Book Class
class Book 
{
    private:
        int id;
        string title;
        string author;
    
    public:
        Book(int id, const string& title, const string& author)
            : id(id), title(title), author(author) {}
    
        int getId() const { return id; }
        const string& getTitle() const { return title; }
        const string& getAuthor() const { return author; }
    
        string serialize() const 
        {
            return to_string(id) + "," + title + "," + author + "\n";
        }
    
        static Book deserialize(const string& data) 
        {
            size_t firstComma = data.find(',');
            size_t secondComma = data.find(',', firstComma + 1);
    
            if (firstComma == string::npos || secondComma == string::npos) 
            {
                throw invalid_argument("Invalid data format.");
            }
    
            int id = stoi(data.substr(0, firstComma));
            string title = data.substr(firstComma + 1, secondComma - firstComma - 1);
            string author = data.substr(secondComma + 1);
    
            return Book(id, title, author);
        }
    
        void display() const 
        {
            cout << "ID: " << id << ", Title: " << title << ", Author: " << author << '\n';
        }
    };
