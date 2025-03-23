#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <map>
#include <sstream>
#include <algorithm>
#include "Book.h"

using namespace std;

// BookRepository Interface
class IBookRepository {
    public:
        virtual void addBook(const string title, const string author) = 0;
        virtual void updateBook(int id, const string& newTitle, const string& newAuthor) = 0;
        virtual void deleteBook(int id) = 0;
        virtual void displayBooks() = 0;
        virtual ~IBookRepository() = default;
    };
    
    // File-Based BookRepository Implementation
    class FileBookRepository : public IBookRepository {
    private:
        string filename;
        string freeIdsFile;

        map<string, string> parseConfigFile(const string& configFile) 
        {
            ifstream inFile(configFile);
            if (!inFile.is_open()) 
            {
                throw runtime_error("Unable to open config file: " + configFile);
            }

            map<string, string> config;
            string line;
            while (getline(inFile, line)) 
            {
                size_t delimiterPos = line.find('=');
                if (delimiterPos != string::npos) 
                {
                    string key = line.substr(0, delimiterPos);
                    string value = line.substr(delimiterPos + 1);
                    config[key] = value;
                }
            }

            inFile.close();
            return config;
        }


        void ensureFile() const 
        {
            fstream file(filename, ios::app);
            if (!file.is_open()) {
                throw runtime_error("Unable to create or open file.");
            }
            file.close();
            chmod(filename.c_str(), S_IRUSR | S_IWUSR); // Read/Write for the owner
        }
    
        vector<Book> loadBooksFromFile() const 
        {
            ensureFile();
            ifstream inFile(filename);
            if (!inFile.is_open()) {
                throw runtime_error("Unable to open file for reading.");
            }
    
            vector<Book> books;
            string line;
            while (getline(inFile, line)) {
                if (!line.empty()) {
                    books.push_back(Book::deserialize(line));
                }
            }
            inFile.close();
            return books;
        }
    
        void saveAllBooksToFile(const vector<Book>& books) const 
        {
            cout << "Saving books to file...\n";
            ofstream outFile(filename, ios::trunc); // Overwrite the file
            if (!outFile.is_open()) 
            {
                throw runtime_error("Unable to open file for writing.");
            }

            for (const auto& book : books) 
            {
                outFile << book.serialize();
            }
            outFile.close();
        }
    
    public:

        explicit FileBookRepository(const string& configFile) 
        {
            auto config = parseConfigFile(configFile);
            if (config.find("BookEntryFile") == config.end() || config.find("FreeIdsFile") == config.end()) 
            {
                throw runtime_error("Missing required config values in " + configFile);
            }
        
            filename = config["BookEntryFile"];
            ensureFile(); // Ensure book file exists
        
            // Free IDs file setup
            freeIdsFile = config["FreeIdsFile"];
            ensureFreeIdsFile(); // Ensure free IDs file exists
        }
        
    
        void ensureFreeIdsFile() const 
        {
            // Open the file in append mode to create it if it doesn't exist
            fstream file(freeIdsFile.c_str(), ios::app);
            if (!file.is_open()) 
            {
                throw runtime_error("Unable to create or open free_ids.txt.");
            }
            file.close();
        
            // Set permissions for read/write by the owner
            chmod(freeIdsFile.c_str(), S_IRUSR | S_IWUSR); // POSIX: Read/Write for the owner
        }
        
    
        void addBook(string title, string author) override 
        {
            int id = fetchFreeId(); // Try to fetch a free ID

            if (id == -1) 
            {   // No free ID available
                vector<Book> books = loadBooksFromFile();
                id = books.empty() ? 1 : books.back().getId() + 1;
                Book newBook(id, title, author);
                books.push_back(newBook);
                saveAllBooksToFile(books); // Save updated list back to the file
            }
            else
            {
                const size_t chunkSize = 100; // Number of lines to process per chunk
                
                cout << "Free ID found: " << id << endl;

                ifstream inFile(filename);
                if (!inFile.is_open()) 
                {
                    throw runtime_error("Unable to open file for reading.");
                }

                bool updated = false;

                while (!inFile.eof()) 
                {
                    vector<Book> books = loadNextChunk(inFile, chunkSize);
                    if(books.empty())
                    {
                        books.push_back(Book(id, title, author));
                        saveAllBooksToFile(books);
                        updated = true;
                        break;
                    }

                    else if(books.front().getId() > id || (books.back().getId() > id && books.front().getId() < id))
                    {
                        books.push_back(Book(id, title, author));
                        sort(books.begin(), books.end(), [](const Book& a, const Book& b) 
                        {
                            return a.getId() < b.getId();
                        });
                        saveAllBooksToFile(books);
                        updated = true;
                        break;
                    }
                }

                if (!updated) 
                {
                    throw runtime_error("Book with the given ID not found.");
                }
                inFile.close();
            }
        } 
    

        vector<Book> loadNextChunk(ifstream& inFile, size_t chunkSize) 
        {
            vector<Book> books; // Store books from the current chunk
            string line;
            size_t lineCount = 0;
        
            while (lineCount < chunkSize && getline(inFile, line)) 
            {
                if (!line.empty()) 
                {
                    books.push_back(Book::deserialize(line)); // Process the line
                }
                lineCount++;
            }
        
            return books;
        }
    
        void updateBook(int id, const string& newTitle, const string& newAuthor) override 
        {
            const size_t chunkSize = 100; // Number of lines to process per chunk
    
            ifstream inFile(filename);
            if (!inFile.is_open()) 
            {
                throw runtime_error("Unable to open file for reading.");
            }

            bool updated = false;

            while (!inFile.eof()) 
            {
                vector<Book> books = loadNextChunk(inFile, chunkSize);

                // Process the chunk (e.g., print, store, etc.)
                for (auto& book : books) 
                {
                    if (book.getId() == id) 
                    {
                        book = Book(id, newTitle, newAuthor); // Update book info
                        updated = true;
                        saveAllBooksToFile(books); // Save updated list back to the file
                        break;
                    }
                }
            }

            if (!updated) 
            {
                throw runtime_error("Book with the given ID not found.");
            }

            inFile.close();
            
        }
    
        void displayBooks() override
        {
            size_t chunkSize = 10;   // Number of books to load in a chunk
            size_t startIndex = 0;   // Starting index for fetching books

            ifstream inFile(filename);
            if (!inFile.is_open()) 
            {
                throw runtime_error("Unable to open file for reading.");
            }

            bool updated = false;

            while (!inFile.eof()) 
            {
                vector<Book> books = loadNextChunk(inFile, chunkSize);

                if (books.empty()) 
                {
                    // If no books are available at all
                    if (startIndex == 0) 
                    {
                        cout << "No books available in the library.\n";
                    } 
                    else 
                    {
                        cout << "This is the end of the books.\n";
                    }
                    break;
                }
    
                // Display the books in the current chunk
                cout << "\nDisplaying books (Chunk " << (startIndex / chunkSize + 1) << "):\n";
                for (const auto& book : books) 
                {
                    book.display();
                }
    
                // Check if we've reached the end of the book data
                if (books.size() < chunkSize) 
                {
                    // Fewer books than the chunk size means end of file
                    cout << "This is the end of the books.\n";
                    break;
                }
    
                // Prompt the user if more books can still be displayed
                cout << "Do you want to display more books? (yes/no): ";
                string response;
                cin >> response;
    
                if (response != "yes") 
                {
                    cout << "You chose to stop. Exiting the display process.\n";
                    break;
                }
    
                // Move to the next chunk
                startIndex += chunkSize;
            }
    
            inFile.close();
        }


        void deleteBook(int id) override 
        {
            const size_t chunkSize = 100; // Number of lines to process per chunk
    
            ifstream inFile(filename);
            if (!inFile.is_open()) 
            {
                throw runtime_error("Unable to open file for reading.");
            }

            bool updated = false;

            while (!inFile.eof()) 
            {
                vector<Book> books = loadNextChunk(inFile, chunkSize);
                auto it = remove_if(books.begin(), books.end(), [id](const Book& book) 
                {
                    return book.getId() == id;
                });

                if (it != books.end()) 
                {
                    books.erase(it, books.end());
                    saveAllBooksToFile(books); // Save the remaining books back to the file
                    addFreeId(id); // Add the deleted book ID to free_ids.txt
                    updated = true;
                    break;
                }
                
            }

            if (updated == false) 
            {
                throw runtime_error("Book with the given ID not found.");
            }

            inFile.close();
        
        }
    
        void addFreeId(int id) const 
        {
            ensureFreeIdsFile(); // Ensure the file exists before appending
            ofstream outFile(freeIdsFile, ios::app); // Open in append mode
            if (!outFile.is_open()) 
            {
                throw runtime_error("Unable to open free_ids.txt for writing.");
            }
            outFile << id << "\n";
            outFile.close();
        }
        
        
    
        int fetchFreeId() const 
        {
            ensureFreeIdsFile(); // Ensure the file exists before reading
            ifstream inFile(freeIdsFile);
            if (!inFile.is_open()) 
            {
                throw runtime_error("Unable to open free_ids.txt for reading.");
            }
        
            vector<int> freeIds;
            string line;
            while (getline(inFile, line)) 
            {
                if (!line.empty()) 
                {
                    freeIds.push_back(stoi(line));
                }
            }
            inFile.close();
        
            if (freeIds.empty()) 
            {
                return -1; // No free ID available
            }
        
            int freeId = freeIds.front();
            // Remove the used free ID from the file
            ofstream outFile(freeIdsFile, ios::trunc); // Overwrite mode
            if (!outFile.is_open()) 
            {
                throw runtime_error("Unable to open free_ids.txt for writing.");
            }

            for (size_t i = 1; i < freeIds.size(); ++i) 
            { // Skip the first (used) ID
                outFile << freeIds[i] << "\n";
            }
            outFile.close();
        
            return freeId;
        }  
    };
    