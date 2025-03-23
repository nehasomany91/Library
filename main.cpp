#include <iostream>
#include <vector>
#include <string>
#include "Library.h"
#include "BookRepository.h"
using namespace std;

// Main Function
int main() 
{
    FileBookRepository repository("config.ini");
    Library library(repository);

    while (true) 
    {
        cout << "\nLibrary Management System:\n";
        cout << "1. Add Book\n";
        cout << "2. Display All Books (in result of 10 entries at a time)\n";
        cout << "3. Update Book Information\n";
        cout << "4. Delete Book Entry\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        if (cin.fail()) 
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) 
        {
            case 1: 
            {
                cin.ignore();
                string title, author;
                cout << "Enter book title: ";
                getline(cin, title);
                cout << "Enter book author: ";
                getline(cin, author);
                library.addBook(title, author);
                break;
            }
            case 2:
                library.displayBooks();
                break;
            
            case 3: 
            {
                int id;
                string newTitle, newAuthor;
                cout << "Enter the book ID to update: ";
                cin >> id;
                cout << "Enter new title: ";
                cin >> newTitle;
                cout << "Enter new author: ";
                cin >> newAuthor;
                library.updateBook(id, newTitle, newAuthor);
            }
            break;
            case 4: 
            {
                int id;
                cout << "Enter the book ID to delete: ";
                cin >> id;
                try {
                    library.deleteBook(id);
                } catch (const exception& e) {
                    cout << "Error: " << e.what() << '\n';
                }
                break;
            }
            case 5:
                return 0;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    }                
    return 0;
}
