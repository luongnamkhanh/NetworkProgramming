#include <bits/stdc++.h>
using namespace std;

// initailize linked list node containing user information (username, password, status)
struct Node{
    string username;
    string password;
    bool status; // 1:active, 0:blocked
    Node *next;
};
int menu(){
    int choice;
    cout<< "USER MANAGEMENT PROGRAM" << endl;
    cout<<  "-----------------------------------" << endl;
    cout << "1. Register" << endl;
    cout << "2. Sign in" << endl;
    cout << "3. Search" << endl;
    cout << "4. Sign out" << endl;
    cout << "Your choice (1-4, other to quit):" << endl;
    cin >> choice;
    return choice;
}

int main(){
    //open file account.txt
    ifstream fin("account.txt");
    //create linked list
    Node *head = NULL;
    Node *tail = NULL;
    //read data from file and add to linked list
    while(!fin.eof()){
        Node *p = new Node;
        fin >> p->username >> p->password >> p->status;
        p->next = NULL;
        if(head == NULL){
            head = p;
            tail = p;
        }
        else{
            tail->next = p;
            tail = p;
        }
    }
    int choice;
    do{
        choice = menu();
        switch(choice){
            case 1:
                //register
                break;
            case 2:
                //sign in
                break;
            case 3:
                //search
                break;
            case 4:
                //sign out
                break;
            default:
                cout << "Goodbye!" << endl;
                break;
        }
    }while(choice >= 1 && choice <= 4);
}