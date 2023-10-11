#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// initailize linked list node containing user information (username, password, status, count)
typedef struct data
{
    char username[20];
    char password[20];
    int status; // 1:active, 0:blocked
    int count;  // number of failed attempts
} account;
struct list_ac //list_account
{
    account data;

    struct list_ac *next;
};
typedef struct list_ac node_ac; // node_account
// Initialisation
node_ac *root, *cur;
node_ac *root = NULL;
node_ac *cur = NULL;
// Function to make new node
node_ac *makeNewNode(account data)
{
    node_ac *newNode = (node_ac *)malloc(sizeof(node_ac));
    if (root == NULL)
    {
        root = newNode;
        cur = newNode;
    }
    else
    {
        cur->next = newNode;
        cur = newNode;
    }

    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}
// free a linked list
void freeList(node_ac *root)
{
    node_ac *cur = root;
    while (cur != NULL)
    {
        node_ac *next = cur->next;
        free(cur);
        cur = next;
    }
}
int menu()
{
    int choice;
    printf("USER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. Register\n");
    printf("2. Sign in\n");
    printf("3. Search\n");
    printf("4. Sign out\n");
    printf("Your choice (1-4, other to quit):\n");
    scanf("%d", &choice);
    return choice;
}
int main()
{
    char username_signed_in[20];
    int choice;
    int sign_in = 0;
    account *arr;
    arr = (account *)malloc(1000 * sizeof(account));

    // open file account.txt
    FILE *f;
    char filename[] = "account.txt";
    if ((f = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open file %s\n", filename);
        return 1;
    }
    // read data from file to linked list
    int i = 0, n;
    while (!feof(f))
    {
        fscanf(f, "%s %s %d", arr[i].username, arr[i].password, &arr[i].status);
        i++;
    }
    n = i;
    for (i = 0; i < n; i++)
    {
        arr[i].count = 0;
        node_ac *newNode = makeNewNode(arr[i]);
    }
    fclose(f);
    char username[20];
    char password[20];
    int flag;
    do
    {
        choice = menu();
        switch (choice)
        {
        case 1:

            printf("Username: \n");
            scanf("%s", username);

            // check if the username is already existed else add new account to linked list
            flag = 0;
            for (i = 0; i < n; i++)
            {
                if (strcmp(arr[i].username, username) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 1)
            {
                printf("Account existed\n");
            }
            else
            {
                printf("Password: \n");
                scanf("%s", password);
                int status = 1; // active
                strcpy(arr[n].username, username); // arr[n] is the new account
                strcpy(arr[n].password, password); 
                arr[n].status = status; 
                arr[n].count = 0; 
                node_ac *newNode = makeNewNode(arr[n]);
                printf("Successful registration\n");
                // write it into file account.txt
                f = fopen(filename, "a");
                fprintf(f, "\n%s %s %d", arr[n].username, arr[n].password, arr[n].status);
                fclose(f);
                n++;
            }
            break;

        case 2:
            // check if any account is signed in
            if (sign_in == 1)
            {
                printf("You have already signed in, you have to sign out first!\n");
                break;
            }
            printf("Username: \n");
            scanf("%s", username);
            // check if the username is existed
            flag = 0;
            for (i = 0; i < n; i++)
            {
                if (strcmp(arr[i].username, username) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            int j = i;
            if (flag == 0)
            {
                printf("Cannot find account\n");
                break;
            }
            printf("Password: \n");
            scanf("%s", password);
            // check if the username and password are correct
            flag = 0;
            for (i = 0; i < n; i++)
            {
                if (strcmp(arr[i].username, username) == 0 && strcmp(arr[i].password, password) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 1 && arr[i].status == 1)
            {
                printf("Hello %s\n", username);
                sign_in = 1;
                strcpy(username_signed_in, username);
                arr[i].count = 0;
            }
            else if (flag == 1 && arr[i].status == 0)
            {
                printf("Account is blocked\n");
            }
            else
            {
                printf("Password is incorrect\n");
                arr[j].count++; // increase the number of wrong password
                if (arr[j].count == 4)
                {
                    arr[j].status = 0;
                    printf("Account is blocked\n");
                    // replace the old data in file account.txt
                    f = fopen(filename, "w");
                    for (i = 0; i < n; i++)
                    {
                        fprintf(f, "%s %s %d\n", arr[i].username, arr[i].password, arr[i].status);
                    }
                    fclose(f);
                }
            }
            break;
        case 3:
            printf("Username: \n");
            scanf("%s", username);
            // search for the username 
            flag = 0;
            for (i = 0; i < n; i++)
            {
                if (strcmp(arr[i].username, username) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            if (sign_in == 0)
            {
                printf("You not signed in yet\n");
            }
            else if (flag == 1 && sign_in == 1)
            {
                if (arr[i].status == 1)
                {
                    printf("Account is active\n");
                }
                else
                {
                    printf("Account is blocked\n");
                }
            }
            else if (flag == 0 && sign_in == 1)
            {
                printf("Cannot find account\n");
            }
            break;
        case 4:
            printf("Username: \n");
            scanf("%s", username);
            // check if the username are correct
            flag = 0;
            for (i = 0; i < n; i++)
            {
                if (strcmp(arr[i].username, username) == 0)
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 1 && sign_in == 1 && strcmp(username_signed_in, username) == 0)
            {
                printf("Goodbye %s\n", username);
                sign_in = 0;
            }
            else if (flag == 1)
            {
                printf("Account is not sign in\n");
            }
            else
            {
                printf("Cannot find account\n");
            }
            break;
        default:
            break;
        }
    } while (choice >= 1 && choice <= 4);
    freeList(root);
    free(arr);
    return 0;
}