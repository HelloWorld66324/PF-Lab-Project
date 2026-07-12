#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACCOUNTS_FILE     "accounts.txt"
#define TRANSACTIONS_FILE "transactions.txt"
#define USERS_FILE        "users.txt"
#define MAX               500
#define MIN_DEPOSIT       1000.0


// STRUCTS


typedef struct {
    int    accNo;
    char   userID[20];
    char   name[50];
    double balance;
} Account;

typedef struct {
    char username[20];
    char password[20];
    char role[10];      //admin" or "user" 
} User;


// ACCOUNT FILE FUNCTIONS  


int loadAccounts(Account list[], int max)
{
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (f == NULL) return 0;
    int n = 0;
    while (n < max) {
        int r = fscanf(f, "%d %s %s %lf",
                       &list[n].accNo, list[n].userID,
                       list[n].name,   &list[n].balance);
        if (r != 4) break;
        n++;
    }
    fclose(f);
    return n;
}

void saveAccounts(Account list[], int total)
{
    FILE *f = fopen(ACCOUNTS_FILE, "w");
    if (f == NULL) { printf("Error saving accounts.\n"); return; }
    int i;
    for (i = 0; i < total; i++)
        fprintf(f, "%d %s %s %.2f\n",
                list[i].accNo, list[i].userID,
                list[i].name,  list[i].balance);
    fclose(f);
}

void saveTransaction(int accNo, char *type, double amount, char *note)
{
    FILE *f = fopen(TRANSACTIONS_FILE, "a");
    if (f == NULL) return;
    fprintf(f, "%d | %s | %.2f | %s\n", accNo, type, amount, note);
    fclose(f);
}

int getNewAccNo()
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);
    if (total == 0) return 1001;
    int highest = 1000, i;
    for (i = 0; i < total; i++)
        if (list[i].accNo > highest) highest = list[i].accNo;
    return highest + 1;
}

int findAcc(Account list[], int total, int accNo)
{
    int i;
    for (i = 0; i < total; i++)
        if (list[i].accNo == accNo) return i;
    return -1;
}


//  USER FILE FUNCTIONS 


int loadUsers(User users[], int max)
{
    FILE *f = fopen(USERS_FILE, "r");
    if (f == NULL) return 0;
    int n = 0;
    while (n < max) {
        int r = fscanf(f, "%s %s %s",
                       users[n].username,
                       users[n].password,
                       users[n].role);
        if (r != 3) break;
        n++;
    }
    fclose(f);
    return n;
}

void saveUsers(User users[], int total)
{
    FILE *f = fopen(USERS_FILE, "w");
    if (f == NULL) return;
    int i;
    for (i = 0; i < total; i++)
        fprintf(f, "%s %s %s\n",
                users[i].username,
                users[i].password,
                users[i].role);
    fclose(f);
}

int findUser(User users[], int total, char *username)
{
    int i;
    for (i = 0; i < total; i++)
        if (strcmp(users[i].username, username) == 0) return i;
    return -1;
}

// First run: create default admin if no users exist
void initAdminIfNeeded()
{
    User users[MAX];
    int total = loadUsers(users, MAX);
    if (total == 0) {
        strcpy(users[0].username, "admin");
        strcpy(users[0].password, "admin123");
        strcpy(users[0].role,     "admin");
        saveUsers(users, 1);
        printf("\n[System] Default admin created.\n");
        printf("         Username : admin\n");
        printf("         Password : admin123\n");
    }
}

   //AUTH FUNCTIONS  (new)

// Returns 1 on success, fills loggedUsername and loggedRole
int loginUser(char *loggedUsername, char *loggedRole)
{
    User users[MAX];
    int total = loadUsers(users, MAX);

    char username[20], password[20];
    printf("\nUsername : "); scanf("%s", username);
    printf("Password : "); scanf("%s", password);

    int pos = findUser(users, total, username);
    if (pos == -1) {
        printf("User not found.\n");
        return 0;
    }
    if (strcmp(users[pos].password, password) != 0) {
        printf("Wrong password.\n");
        return 0;
    }

    strcpy(loggedUsername, users[pos].username);
    strcpy(loggedRole,     users[pos].role);
    printf("Login successful! Welcome, %s.\n", loggedUsername);
    return 1;
}

void signupUser()
{
    User users[MAX];
    int total = loadUsers(users, MAX);

    char username[20], password[20];
    printf("\nChoose Username : "); scanf("%s", username);

    if (findUser(users, total, username) != -1) {
        printf("Username already taken. Try another.\n");
        return;
    }

    printf("Choose Password : "); scanf("%s", password);

    strcpy(users[total].username, username);
    strcpy(users[total].password, password);
    strcpy(users[total].role,     "user");
    total++;

    saveUsers(users, total);
    printf("Signup successful! You can now login.\n");
}


// ADMIN FUNCTIONS

void adminCreateAccount()
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    Account a;
    a.accNo = getNewAccNo();

    printf("\nEnter User ID (their login username) : "); scanf("%s", a.userID);
    printf("Enter Full Name                       : "); scanf("%s", a.name);

    double dep;
    printf("Opening Deposit (min Rs.%.0f)         : ", MIN_DEPOSIT);
    scanf("%lf", &dep);

    if (dep < MIN_DEPOSIT) {
        printf("Deposit below minimum. Account not created.\n");
        return;
    }

    a.balance    = dep;
    list[total]  = a;
    total++;

    saveAccounts(list, total);
    saveTransaction(a.accNo, "Account Opened", dep, "Opening deposit by admin");
    printf("Account created! Account Number: %d\n", a.accNo);
}

void adminDeleteAccount()
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    int accNo;
    printf("\nEnter Account Number to delete: "); scanf("%d", &accNo);

    int pos = findAcc(list, total, accNo);
    if (pos == -1) { printf("Account not found.\n"); return; }

    printf("Delete account of '%s'? (1=Yes 0=No): ", list[pos].name);
    int confirm; scanf("%d", &confirm);
    if (!confirm) { printf("Cancelled.\n"); return; }

    int deleted = list[pos].accNo;
    int i;
    for (i = pos; i < total - 1; i++)
        list[i] = list[i + 1];
    total--;

    saveAccounts(list, total);
    saveTransaction(deleted, "Account Closed", 0, "Deleted by admin");
    printf("Account deleted.\n");
}

void adminViewAllAccounts()
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    if (total == 0) { printf("No accounts found.\n"); return; }

    printf("\n%-8s %-15s %-20s %s\n", "AccNo", "UserID", "Name", "Balance");
    printf("----------------------------------------------------------\n");
    int i;
    for (i = 0; i < total; i++)
        printf("%-8d %-15s %-20s Rs.%.2f\n",
               list[i].accNo, list[i].userID,
               list[i].name,  list[i].balance);
    printf("----------------------------------------------------------\n");
    printf("Total Accounts: %d\n", total);
}

void adminSearchAccount()
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    printf("\nSearch by:\n1. Account Number\n2. Name\n3. User ID\nChoice: ");
    int choice; scanf("%d", &choice);

    int found = 0, i;

    if (choice == 1) {
        int accNo;
        printf("Enter Account Number: "); scanf("%d", &accNo);
        for (i = 0; i < total; i++)
            if (list[i].accNo == accNo) {
                printf("Acc: %d  Name: %s  ID: %s  Balance: Rs.%.2f\n",
                       list[i].accNo, list[i].name,
                       list[i].userID, list[i].balance);
                found = 1;
            }
    } else if (choice == 2) {
        char name[50];
        printf("Enter Name: "); scanf("%s", name);
        for (i = 0; i < total; i++)
            if (strcasecmp(list[i].name, name) == 0) {
                printf("Acc: %d  Name: %s  ID: %s  Balance: Rs.%.2f\n",
                       list[i].accNo, list[i].name,
                       list[i].userID, list[i].balance);
                found = 1;
            }
    } else if (choice == 3) {
        char uid[20];
        printf("Enter User ID: "); scanf("%s", uid);
        for (i = 0; i < total; i++)
            if (strcmp(list[i].userID, uid) == 0) {
                printf("Acc: %d  Name: %s  ID: %s  Balance: Rs.%.2f\n",
                       list[i].accNo, list[i].name,
                       list[i].userID, list[i].balance);
                found = 1;
            }
    } else {
        printf("Invalid choice.\n"); return;
    }

    if (!found) printf("No account found.\n");
}

// Admin can see all transactions OR filter by account 
void adminTransactionHistory()
{
    int accNo;
    printf("\nEnter Account Number (0 = show all): "); scanf("%d", &accNo);

    FILE *f = fopen(TRANSACTIONS_FILE, "r");
    if (f == NULL) { printf("No transactions found.\n"); return; }

    char line[256];
    int found = 0;
    printf("\n%-10s %-22s %-14s %s\n", "AccNo", "Type", "Amount", "Note");
    printf("----------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), f)) {
        int acc; char type[30], note[100]; double amt;
        int r = sscanf(line, "%d | %29[^|] | %lf | %99[^\n]",
                       &acc, type, &amt, note);
        if (r == 4 && (accNo == 0 || acc == accNo)) {
            printf("%-10d %-22s Rs.%-10.2f %s\n", acc, type, amt, note);
            found = 1;
        }
    }
    fclose(f);
    if (!found) printf("No transactions found.\n");
}

void adminUpdateAccount()
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    int accNo;
    printf("\nEnter Account Number: "); scanf("%d", &accNo);

    int pos = findAcc(list, total, accNo);
    if (pos == -1) { printf("Account not found.\n"); return; }

    printf("1. Change Name\n2. Change User ID\nChoice: ");
    int choice; scanf("%d", &choice);

    if (choice == 1) {
        printf("New Name: "); scanf("%s", list[pos].name);
        printf("Name updated.\n");
    } else if (choice == 2) {
        printf("New User ID: "); scanf("%s", list[pos].userID);
        printf("User ID updated.\n");
    } else {
        printf("Invalid choice.\n"); return;
    }
    saveAccounts(list, total);
}

// USER FUNCTIONS

// Security helper 
int ownsAccount(Account list[], int pos, char *uid)
{
    if (strcmp(list[pos].userID, uid) != 0) {
        printf("Access denied. This account does not belong to you.\n");
        return 0;
    }
    return 1;
}

/* 
    KEY HELPER  selectMyAccount()

   Ye function:
   1. uid ke saare accounts dhoondhta hai
   2. Numbered list print karta hai (user ko accNo yaad nahi karna)
   3. User se number choose karwata hai
   4. Agar sirf ek account hai to automatically select karta hai
   5. Chosen account ka actual accNo return karta hai
   6. Cancel ya koi account na ho to -1 return karta hai
*/

int selectMyAccount(char *uid)
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    int matches[MAX], count = 0, i;
    for (i = 0; i < total; i++)
        if (strcmp(list[i].userID, uid) == 0)
            matches[count++] = i;

    if (count == 0) {
        printf("No accounts found. Please contact admin to open one.\n");
        return -1;
    }

    printf("\n--- Your Accounts ---\n");
    printf("%-4s %-10s %-20s %s\n", "No", "AccNo", "Name", "Balance");
    printf("------------------------------------------\n");
    for (i = 0; i < count; i++)
        printf("%-4d %-10d %-20s Rs.%.2f\n",
               i + 1,
               list[matches[i]].accNo,
               list[matches[i]].name,
               list[matches[i]].balance);
    printf("------------------------------------------\n");

    int pick;
    if (count == 1) {
        // Sirf ek account  automatically select 
        pick = 1;
        printf("Auto-selected: Account %d\n", list[matches[0]].accNo);
    } else {
        printf("Select account (1-%d), 0 to cancel: ", count);
        scanf("%d", &pick);
        if (pick < 1 || pick > count) {
            printf("Cancelled.\n");
            return -1;
        }
    }

    return list[matches[pick - 1]].accNo;  // actual accNo return 
}

void userViewMyAccounts(char *uid)
{
    Account list[MAX];
    int total = loadAccounts(list, MAX);

    int matches[MAX], count = 0, i;
    for (i = 0; i < total; i++)
        if (strcmp(list[i].userID, uid) == 0)
            matches[count++] = i;

    if (count == 0) { printf("No accounts found. Please contact admin.\n"); return; }

    printf("\n%-4s %-10s %-20s %s\n", "No", "AccNo", "Name", "Balance");
    printf("------------------------------------------\n");
    for (i = 0; i < count; i++)
        printf("%-4d %-10d %-20s Rs.%.2f\n",
               i + 1,
               list[matches[i]].accNo,
               list[matches[i]].name,
               list[matches[i]].balance);
    printf("------------------------------------------\n");
    printf("Total: %d account(s)\n", count);
}

void userDeposit(char *uid)
{
    // Pehle accounts dikhao, user khud select kare 
    int accNo = selectMyAccount(uid);
    if (accNo == -1) return;

    Account list[MAX];
    int total = loadAccounts(list, MAX);
    int pos   = findAcc(list, total, accNo);
    if (pos == -1)                    { printf("Error loading account.\n"); return; }
    if (!ownsAccount(list, pos, uid)) return;

    double amount;
    printf("Enter amount to deposit: Rs. "); scanf("%lf", &amount);
    if (amount <= 0) { printf("Invalid amount.\n"); return; }

    list[pos].balance += amount;
    saveAccounts(list, total);
    saveTransaction(list[pos].accNo, "Deposit", amount, "Cash deposit");
    printf("Rs.%.2f deposited. New balance: Rs.%.2f\n", amount, list[pos].balance);
}

void userWithdraw(char *uid)
{
    int accNo = selectMyAccount(uid);
    if (accNo == -1) return;

    Account list[MAX];
    int total = loadAccounts(list, MAX);
    int pos   = findAcc(list, total, accNo);
    if (pos == -1)                    { printf("Error loading account.\n"); return; }
    if (!ownsAccount(list, pos, uid)) return;

    double amount;
    printf("Enter amount to withdraw: Rs. "); scanf("%lf", &amount);
    if (amount <= 0)                { printf("Invalid amount.\n"); return; }
    if (amount > list[pos].balance) { printf("Not enough balance.\n"); return; }

    if ((list[pos].balance - amount) < MIN_DEPOSIT) {
        printf("Warning: balance will go below Rs.%.0f\n", MIN_DEPOSIT);
        printf("Continue? (1=Yes 0=No): ");
        int go; scanf("%d", &go);
        if (!go) { printf("Cancelled.\n"); return; }
    }

    list[pos].balance -= amount;
    saveAccounts(list, total);
    saveTransaction(list[pos].accNo, "Withdraw", amount, "Cash withdrawal");
    printf("Rs.%.2f withdrawn. New balance: Rs.%.2f\n", amount, list[pos].balance);
}

void userSendMoney(char *uid)
{
    // SENDER  apni list se select karo
    printf("\n-- Select YOUR account to send FROM --\n");
    int fromAcc = selectMyAccount(uid);
    if (fromAcc == -1) return;

    Account list[MAX];
    int total   = loadAccounts(list, MAX);
    int fromPos = findAcc(list, total, fromAcc);
    if (fromPos == -1)                    { printf("Error loading account.\n"); return; }
    if (!ownsAccount(list, fromPos, uid)) return;

    // RECEIVER  accNo manually dalo (doosre ka account)
    int toAcc;
    printf("Enter Destination Account Number: "); scanf("%d", &toAcc);

    int toPos = findAcc(list, total, toAcc);
    if (toPos == -1)      { printf("Destination account not found.\n"); return; }
    if (fromAcc == toAcc) { printf("Cannot send to same account.\n"); return; }

    double amount;
    printf("Sending to: %s\n", list[toPos].name);
    printf("Enter amount to send: Rs. "); scanf("%lf", &amount);
    if (amount <= 0)                    { printf("Invalid amount.\n"); return; }
    if (amount > list[fromPos].balance) { printf("Not enough balance.\n"); return; }

    list[fromPos].balance -= amount;
    list[toPos].balance   += amount;
    saveAccounts(list, total);

    char note[100];
    sprintf(note, "Sent to account %d", toAcc);
    saveTransaction(fromAcc, "Transfer Out", amount, note);
    sprintf(note, "Received from account %d", fromAcc);
    saveTransaction(toAcc, "Transfer In", amount, note);

    printf("Rs.%.2f sent to %s.\n", amount, list[toPos].name);
    printf("Your remaining balance: Rs.%.2f\n", list[fromPos].balance);
}

void userPayBill(char *uid)
{
    int accNo = selectMyAccount(uid);
    if (accNo == -1) return;

    Account list[MAX];
    int total = loadAccounts(list, MAX);
    int pos   = findAcc(list, total, accNo);
    if (pos == -1)                    { printf("Error loading account.\n"); return; }
    if (!ownsAccount(list, pos, uid)) return;

    printf("Select Bill Type:\n");
    printf("1. Electricity\n2. Gas\n3. Water\n4. Internet\nChoice: ");
    int choice; scanf("%d", &choice);

    char *bills[] = {"Electricity", "Gas", "Water", "Internet"};
    if (choice < 1 || choice > 4) { printf("Invalid choice.\n"); return; }

    double amount;
    printf("Enter bill amount: Rs. "); scanf("%lf", &amount);
    if (amount <= 0)                { printf("Invalid amount.\n"); return; }
    if (amount > list[pos].balance) { printf("Not enough balance.\n"); return; }

    list[pos].balance -= amount;
    saveAccounts(list, total);

    char note[100];
    sprintf(note, "%s bill paid", bills[choice - 1]);
    saveTransaction(list[pos].accNo, "Bill Payment", amount, note);

    printf("%s bill of Rs.%.2f paid.\n", bills[choice - 1], amount);
    printf("Remaining balance: Rs.%.2f\n", list[pos].balance);
}

void userStatement(char *uid)
{
    int accNo = selectMyAccount(uid);
    if (accNo == -1) return;

    Account list[MAX];
    int total = loadAccounts(list, MAX);
    int pos   = findAcc(list, total, accNo);
    if (pos == -1)                    { printf("Error loading account.\n"); return; }
    if (!ownsAccount(list, pos, uid)) return;

    printf("\n========== Account Statement ==========\n");
    printf("Account No : %d\n",      list[pos].accNo);
    printf("Name       : %s\n",      list[pos].name);
    printf("ID         : %s\n",      list[pos].userID);
    printf("Balance    : Rs.%.2f\n", list[pos].balance);
    printf("---------------------------------------\n");
    printf("%-22s %-14s %s\n", "Type", "Amount", "Note");
    printf("---------------------------------------\n");

    FILE *f = fopen(TRANSACTIONS_FILE, "r");
    if (f == NULL) { printf("No transactions found.\n"); return; }

    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        int acc; char type[30], note[100]; double amt;
        int r = sscanf(line, "%d | %29[^|] | %lf | %99[^\n]",
                       &acc, type, &amt, note);
        if (r == 4 && acc == list[pos].accNo) {
            printf("%-22s Rs.%-10.2f %s\n", type, amt, note);
            found = 1;
        }
    }
    fclose(f);
    if (!found) printf("No transactions on record.\n");
    printf("=======================================\n");
}

void userUpdateAccount(char *uid)
{
    int accNo = selectMyAccount(uid);
    if (accNo == -1) return;

    Account list[MAX];
    int total = loadAccounts(list, MAX);
    int pos   = findAcc(list, total, accNo);
    if (pos == -1)                    { printf("Error loading account.\n"); return; }
    if (!ownsAccount(list, pos, uid)) return;

    printf("1. Change Name\nChoice: ");
    int choice; scanf("%d", &choice);

    if (choice == 1) {
        printf("New Name: "); scanf("%s", list[pos].name);
        printf("Name updated.\n");
    } else {
        printf("Invalid choice.\n"); return;
    }
    saveAccounts(list, total);
}


// MENUS Section

void adminMenu()
{
    int choice;
    do {
        printf("\n========== ADMIN PANEL ==========\n");
        printf("1. Create Account\n");
        printf("2. Delete Account\n");
        printf("3. View All Accounts\n");
        printf("4. Search Account\n");
        printf("5. Transaction History\n");
        printf("6. Update Account\n");
        printf("0. Logout\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: adminCreateAccount();      break;
            case 2: adminDeleteAccount();      break;
            case 3: adminViewAllAccounts();    break;
            case 4: adminSearchAccount();      break;
            case 5: adminTransactionHistory(); break;
            case 6: adminUpdateAccount();      break;
            case 0: printf("Admin logged out.\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

void userMenu(char *username)
{
    int choice;
    do {
        printf("\n========== USER PANEL ==========\n");
        printf("Logged in as: %s\n", username);
        printf("1. View My Accounts\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Send Money\n");
        printf("5. Pay Utility Bill\n");
        printf("6. Account Statement\n");
        printf("0. Logout\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: userViewMyAccounts(username); break;
            case 2: userDeposit(username);        break;
            case 3: userWithdraw(username);       break;
            case 4: userSendMoney(username);      break;
            case 5: userPayBill(username);        break;
            case 6: userStatement(username);      break;
            case 0: printf("Logged out.\n");      break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}


// MAIN

int main()
{
    initAdminIfNeeded();   // Create default admin on first run 

    printf("============ BANK MANAGEMENT SYSTEM ============\n");

    int choice;
    do {
        printf("\n1. Login\n");
        printf("2. Signup\n");
        printf("0. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            char username[20], role[10];
            if (loginUser(username, role)) {
                if (strcmp(role, "admin") == 0)
                    adminMenu();
                else
                    userMenu(username);
            }
        } else if (choice == 2) {
            signupUser();
        } else if (choice == 0) {
            printf("Goodbye!\n");
        } else {
            printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
