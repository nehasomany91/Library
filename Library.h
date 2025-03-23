#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "BookRepository.h"

using namespace std;

// Library Class
class Library 
{
    private:
        IBookRepository& bookRepository;
    
    public:
        explicit Library(IBookRepository& repository) : bookRepository(repository) 
        {
        }
    
        void addBook(const string& title, const string& author) 
        {
            bookRepository.addBook(title, author);
            cout << "Book added successfully.\n";
        }
    
        void displayBooks() const 
        {
            try
            {
                bookRepository.displayBooks();
            }
            catch(const exception& e)
            {
                cout << "Error: " << e.what() << '\n';
            }
            
        }
    
        void updateBook(int id, const string& newTitle, const string& newAuthor) 
        {
            try 
            {
                bookRepository.updateBook(id, newTitle, newAuthor);
                cout << "Book updated successfully.\n";
            } 
            catch (const exception& e) 
            {
                cout << "Error: " << e.what() << '\n';
            }
        }
    
        void deleteBook(int id) 
        {
            try 
            {
                bookRepository.deleteBook(id);
                cout << "Book deleted successfully.\n";
            } 
            catch (const exception& e) 
            {
                cout << "Error: " << e.what() << '\n';
            }
        }
    };
    