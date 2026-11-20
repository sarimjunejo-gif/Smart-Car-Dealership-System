/* ======================================================================
   SMART CAR SHOWROOM MANAGEMENT SYSTEM  




   
   ====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------------------------- 1. CONSTANTS --------------------------- */
#define CAR_CATEGORIES   10
#define CAR_COMPANIES     5
#define CAR_VARIANTS      3
#define MAX_CARS        200
#define MAX_CART_INIT     5          /* cart starts with 5 slots         */
#define MAX_CART_GROW     5          /* and grows by 5 when full          */
#define CUST_INIT_ROOM   10          /* customer list starts with 10      */
#define CUST_GROW_ROOM   10          /* and grows by 10 when full         */
#define MAX_CUSTOMERS   200          /* hard safety ceiling               */
#define NAME_LENGTH      30
#define PHONE_LENGTH     15
#define DATE_LENGTH      12
#define LINE_SIZE       100
#define ADMIN_PIN      1234
#define PIN_ATTEMPTS      3
#define DATA_FILE   "showroom_data.txt"

/* named numbers */
#define PAY_CASH          1
#define PAY_LOAN          2
#define PAY_LEASE         3
#define STATUS_UNPAID     1
#define STATUS_PARTLY     2
#define STATUS_PAID       3
#define SORT_BY_NAME      1
#define SORT_BY_ID        2
#define SORT_BY_BILL      3

/* business rules (percent) */
#define DISCOUNT_SENIOR   5
#define DISCOUNT_LOYAL    3
#define DISCOUNT_MULTI    4
#define DISCOUNT_CASH     2
#define CHARGE_LOAN      10
#define CHARGE_LEASE      6
#define SENIOR_AGE       60

/* ------------------------------ 2. DATA ------------------------------ */
char category_names[CAR_CATEGORIES][NAME_LENGTH] = {"Hatchback", "Sedan", "SUV", "Crossover",
    "Coupe", "Convertible", "Pickup Truck", "Van/MPV", "Electric", "Luxury"};
char company_names[CAR_COMPANIES][NAME_LENGTH] = {"Toyota", "Honda", "Suzuki", "Hyundai", "Kia"};
char variant_names[CAR_VARIANTS][NAME_LENGTH] = {"Base", "Mid", "Top"};
char customer_type_names[5][NAME_LENGTH] = {"Urgent Delivery", "VIP", "Corporate", "Pre-booked", "Normal"};
char payment_names[3][NAME_LENGTH] = {"Cash", "Bank Loan", "Lease"};
char status_names[3][NAME_LENGTH] = {"Unpaid", "Partly Paid", "Paid"};

double variant_factor[CAR_VARIANTS] = {1.00, 1.15, 1.30};   /* Base, Mid, Top */

/* base price in lakh: rows = category, columns = company */
double price_table[CAR_CATEGORIES][CAR_COMPANIES] = {
    {38, 35, 25, 30, 32},    {55, 52, 40, 48, 50},    {95, 88, 65, 80, 85},
    {70, 68, 50, 62, 66},    {120, 115, 90, 105, 110}, {150, 145, 110, 130, 140},
    {85, 78, 55, 70, 75},    {60, 58, 42, 52, 56},    {110, 105, 80, 95, 100},
    {220, 210, 160, 190, 200}};

typedef struct
{
    int id;
    char name[NAME_LENGTH];
    char phone[PHONE_LENGTH];
    int age;
    int type;                  /* 1..5, 1 = most urgent */
    int previous_purchases;
    double current_bill;
    double amount_paid;
    int payment_type;
    int payment_status;
    char registration_date[DATE_LENGTH];
} Customer;

typedef struct
{
    int id;
    int category;
    int company;
    char model[NAME_LENGTH];
    int variant;
    int stock;
} Car;

typedef struct
{
    int car_id;
    int quantity;
} CartItem;

/* dynamic (malloc/realloc) — the customer list */
Customer *customers = NULL;
int customer_count = 0;
int customer_room  = 0;          /* how many slots we have allocated */

Car cars[MAX_CARS];
int car_count = 0;

/* dynamic (malloc/realloc) — the shopping cart */
CartItem *cart = NULL;
int cart_count = 0;
int cart_room  = 0;

int next_customer_id = 1001;
int next_car_id = 2001;

char today_date[DATE_LENGTH];
int admin_locked = 0;


/* ======================================================================
   3. INPUT HELPERS  (safe keyboard input)
   ====================================================================== */

int read_line(char line[], int size)
{
    int length, ch;

    if (fgets(line, size, stdin) == NULL)
        return 0;
    length = strlen(line);
    if (length > 0 && line[length - 1] == '\n')
        line[length - 1] = '\0';
    else
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    return 1;
}

int read_int(char prompt[], int min, int max)
{
    char line[LINE_SIZE];
    char extra;
    int value;

    while (1)
    {
        printf("%s", prompt);
        if (read_line(line, LINE_SIZE) == 0)
            return min;
        if (sscanf(line, "%d %c", &value, &extra) != 1)
            printf("  Invalid input. Please type a whole number.\n");
        else if (value < min || value > max)
            printf("  Number must be between %d and %d.\n", min, max);
        else
            return value;
    }
}

double read_double(char prompt[], double min, double max)
{
    char line[LINE_SIZE];
    char extra;
    double value;

    while (1)
    {
        printf("%s", prompt);
        if (read_line(line, LINE_SIZE) == 0)
            return min;
        if (sscanf(line, "%lf %c", &value, &extra) != 1)
            printf("  Invalid input. Please type a number.\n");
        else if (value < min || value > max + 0.001)   /* small tolerance for decimal rounding */
            printf("  Amount must be between %.2f and %.2f.\n", min, max);
        else
            return value;
    }
}

int read_text(char prompt[], char dest[], int max_length)
{
    char line[LINE_SIZE];
    int start, length, i;

    while (1)
    {
        printf("%s", prompt);
        if (read_line(line, LINE_SIZE) == 0)
            return 0;
        start = 0;
        while (line[start] == ' ')
            start++;
        length = strlen(line + start);
        while (length > 0 && line[start + length - 1] == ' ')
            length--;
        line[start + length] = '\0';
        for (i = start; i < start + length; i++)
            if (line[i] == ' ')
                line[i] = '_';

        if (length == 0)
            printf("  Text cannot be empty.\n");
        else if (length >= max_length)
            printf("  Too long. Maximum %d characters.\n", max_length - 1);
        else
        {
            strcpy(dest, line + start);
            return 1;
        }
    }
}

int same_text(char a[], char b[])
{
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0' && tolower((unsigned char)a[i]) == tolower((unsigned char)b[i]))
        i++;
    return a[i] == b[i];
}

void pause_screen(void)
{
    char line[LINE_SIZE];

    printf("\nPress Enter to continue...");
    read_line(line, LINE_SIZE);
}

void show_list(char title[], char names[][NAME_LENGTH], int count)
{
    int i;

    printf("\n%s:\n", title);
    for (i = 0; i < count; i++)
        printf("  %d. %s\n", i + 1, names[i]);
}

/* visual helpers — only printf, nothing new */
void print_rule(char c, int n)
{
    int i;
    for (i = 0; i < n; i++)
        putchar(c);
    putchar('\n');
}

void print_title(char title[])
{
    int len = strlen(title);
    int pad = (60 - len) / 2;
    int i;

    print_rule('=', 60);
    printf("%*s%s\n", pad > 0 ? pad : 0, "", title);
    print_rule('=', 60);
}


/* ======================================================================
   4. CUSTOMERS  (with malloc/realloc growth)
   ====================================================================== */

/* Make sure we have at least one free slot. Returns 1 on success. */
int customer_ensure_room(void)
{
    Customer *temp;

    if (customer_count < customer_room)
        return 1;
    if (customer_room >= MAX_CUSTOMERS)
        return 0;
    customer_room += CUST_GROW_ROOM;
    if (customer_room > MAX_CUSTOMERS)
        customer_room = MAX_CUSTOMERS;
    temp = (Customer *) realloc(customers, customer_room * sizeof(Customer));
    if (temp == NULL)
        return 0;
    customers = temp;
    return 1;
}

int find_customer(int id)
{
    int i;

    for (i = 0; i < customer_count; i++)
        if (customers[i].id == id)
            return i;
    return -1;
}

int ask_customer_position(void)
{
    int id = read_int("Enter customer ID: ", 1, 999999);
    int pos = find_customer(id);

    if (pos < 0)
        printf("  Customer ID %d was not found.\n", id);
    return pos;
}

void customer_print_header(void)
{
    printf("\n%-6s %-18s %-14s %-4s %-16s %-5s %-10s %-10s %s\n",
           "ID", "Name", "Phone", "Age", "Type", "Buys", "Bill", "Paid", "Status");
    print_rule('-', 92);
}

void customer_print_row(int pos)
{
    printf("%-6d %-18s %-14s %-4d %-16s %-5d %-10.2f %-10.2f %s\n",
           customers[pos].id, customers[pos].name, customers[pos].phone, customers[pos].age,
           customer_type_names[customers[pos].type - 1], customers[pos].previous_purchases,
           customers[pos].current_bill, customers[pos].amount_paid,
           status_names[customers[pos].payment_status - 1]);
}

int customer_register(void)
{
    Customer c;

    if (!customer_ensure_room())
    {
        printf("Customer list is full.\n");
        return 0;
    }
    printf("\n--- Register New Customer ---\n");
    c.id = next_customer_id;
    if (read_text("Enter name: ", c.name, NAME_LENGTH) == 0 ||
        read_text("Enter phone number: ", c.phone, PHONE_LENGTH) == 0)
        return 0;
    c.age = read_int("Enter age (1-120): ", 1, 120);
    show_list("Customer types", customer_type_names, 5);
    c.type = read_int("Enter customer type number: ", 1, 5);
    c.previous_purchases = 0;
    c.current_bill = 0;
    c.amount_paid = 0;
    c.payment_type = PAY_CASH;
    c.payment_status = STATUS_PAID;
    strcpy(c.registration_date, today_date);

    customers[customer_count] = c;
    customer_count++;
    next_customer_id++;
    printf("Customer registered. Customer ID = %d\n", c.id);
    return 1;
}

int customer_update(int id)
{
    int pos = find_customer(id);
    int choice;

    if (pos == -1)
    {
        printf("Customer with ID %d was not found.\n", id);
        return 0;
    }
    do
    {
        customer_print_header();
        customer_print_row(pos);
        printf("\n1. Change name\n2. Change phone\n3. Change age\n4. Change type\n0. Back\n");
        choice = read_int("Enter your choice: ", 0, 4);
        switch (choice)
        {
            case 1: read_text("New name: ", customers[pos].name, NAME_LENGTH); break;
            case 2: read_text("New phone: ", customers[pos].phone, PHONE_LENGTH); break;
            case 3: customers[pos].age = read_int("New age (1-120): ", 1, 120); break;
            case 4:
                show_list("Customer types", customer_type_names, 5);
                customers[pos].type = read_int("New type number: ", 1, 5);
                break;
        }
    } while (choice != 0);
    return 1;
}

int customer_delete(int id)
{
    int pos = find_customer(id);
    int i;

    if (pos == -1)
    {
        printf("Customer with ID %d was not found.\n", id);
        return 0;
    }
    if (customers[pos].payment_status != STATUS_PAID)
    {
        printf("Cannot delete: this customer has an unpaid bill.\n");
        return 0;
    }
    customer_print_header();
    customer_print_row(pos);
    if (read_int("Delete this customer? (1 = Yes, 2 = No): ", 1, 2) == 2)
    {
        printf("Delete cancelled.\n");
        return 0;
    }
    for (i = pos; i < customer_count - 1; i++)
        customers[i] = customers[i + 1];
    customer_count--;
    printf("Customer deleted.\n");
    return 1;
}

void customer_list_all(void)
{
    int i;

    if (customer_count == 0)
    {
        printf("\nNo customers registered yet.\n");
        return;
    }
    printf("\n--- All Customers (%d) ---", customer_count);
    customer_print_header();
    for (i = 0; i < customer_count; i++)
        customer_print_row(i);
}


/* ======================================================================
   5. CARS
   ====================================================================== */

double price_get(int category, int company, int variant)
{
    return price_table[category - 1][company - 1] * variant_factor[variant - 1];
}

int find_car(int id)
{
    int i;

    for (i = 0; i < car_count; i++)
        if (cars[i].id == id)
            return i;
    return -1;
}

void car_print_header(void)
{
    printf("\n%-6s %-14s %-10s %-16s %-8s %-12s %s\n",
           "ID", "Category", "Company", "Model", "Variant", "Price(lakh)", "Stock");
    print_rule('-', 83);
}

void car_print_row(int pos)
{
    printf("%-6d %-14s %-10s %-16s %-8s %-12.2f %d\n",
           cars[pos].id, category_names[cars[pos].category - 1], company_names[cars[pos].company - 1],
           cars[pos].model, variant_names[cars[pos].variant - 1],
           price_get(cars[pos].category, cars[pos].company, cars[pos].variant), cars[pos].stock);
}

int car_add(void)
{
    Car c;
    int i;

    if (car_count >= MAX_CARS)
    {
        printf("Car list is full.\n");
        return 0;
    }
    printf("\n--- Add New Car ---\n");
    c.id = next_car_id;
    show_list("Categories", category_names, CAR_CATEGORIES);
    c.category = read_int("Enter category number: ", 1, CAR_CATEGORIES);
    show_list("Companies", company_names, CAR_COMPANIES);
    c.company = read_int("Enter company number: ", 1, CAR_COMPANIES);
    if (read_text("Enter model name: ", c.model, NAME_LENGTH) == 0)
        return 0;
    show_list("Variants", variant_names, CAR_VARIANTS);
    c.variant = read_int("Enter variant number: ", 1, CAR_VARIANTS);
    c.stock = read_int("Enter stock (0-1000): ", 0, 1000);

    for (i = 0; i < car_count; i++)
        if (cars[i].category == c.category && cars[i].company == c.company &&
            cars[i].variant == c.variant && same_text(cars[i].model, c.model))
        {
            printf("This car already exists (ID %d). Use update to change its stock.\n", cars[i].id);
            return 0;
        }
    cars[car_count] = c;
    car_count++;
    next_car_id++;
    printf("Car added. Car ID = %d, Price = %.2f lakh\n", c.id, price_get(c.category, c.company, c.variant));
    return 1;
}

int car_update(int id)
{
    int pos = find_car(id);
    int choice;

    if (pos == -1)
    {
        printf("Car with ID %d was not found.\n", id);
        return 0;
    }
    do
    {
        car_print_header();
        car_print_row(pos);
        printf("\n1. Change model name\n2. Change variant\n3. Change stock\n");
        printf("4. Change base price (for this category and company)\n0. Back\n");
        choice = read_int("Enter your choice: ", 0, 4);
        switch (choice)
        {
            case 1: read_text("New model name: ", cars[pos].model, NAME_LENGTH); break;
            case 2:
                show_list("Variants", variant_names, CAR_VARIANTS);
                cars[pos].variant = read_int("New variant number: ", 1, CAR_VARIANTS);
                break;
            case 3: cars[pos].stock = read_int("New stock (0-1000): ", 0, 1000); break;
            case 4:
                price_table[cars[pos].category - 1][cars[pos].company - 1] =
                    read_int("New base price in lakh (1-1000): ", 1, 1000);
                printf("Base price updated for all cars of this category and company.\n");
                break;
        }
    } while (choice != 0);
    return 1;
}

void car_browse(int category)
{
    int i, found = 0;

    car_print_header();
    for (i = 0; i < car_count; i++)
        if (category == 0 || cars[i].category == category)
        {
            car_print_row(i);
            found++;
        }
    if (found == 0)
        printf("No cars found.\n");
}


/* ======================================================================
   6. SEARCHING AND SORTING
   ====================================================================== */

/* Recursive binary search — customers are always kept in ID order */
int search_customer_by_id_rec(int id, int low, int high)
{
    int mid;

    if (low > high)
        return -1;
    mid = (low + high) / 2;
    if (customers[mid].id == id)
        return mid;
    if (id < customers[mid].id)
        return search_customer_by_id_rec(id, low, mid - 1);
    return search_customer_by_id_rec(id, mid + 1, high);
}

int search_customer_by_id(int id)
{
    return search_customer_by_id_rec(id, 0, customer_count - 1);
}

void search_customer_by_name(void)
{
    char text[NAME_LENGTH];
    int i, found = 0;

    read_text("Enter customer name to search: ", text, NAME_LENGTH);
    for (i = 0; i < customer_count; i++)
        if (same_text(customers[i].name, text))
        {
            if (found == 0)
                customer_print_header();
            customer_print_row(i);
            found++;
        }
    if (found == 0)
        printf("No customer found with that name.\n");
}

void search_car_by_model(void)
{
    char text[NAME_LENGTH];
    int i, found = 0;

    read_text("Enter car model to search: ", text, NAME_LENGTH);
    for (i = 0; i < car_count; i++)
        if (same_text(cars[i].model, text))
        {
            if (found == 0)
                car_print_header();
            car_print_row(i);
            found++;
        }
    if (found == 0)
        printf("No car found with that model.\n");
}

int customer_is_after(Customer a, Customer b, int sort_by)
{
    if (sort_by == SORT_BY_NAME)
        return strcmp(a.name, b.name) > 0;
    if (sort_by == SORT_BY_BILL)
        return a.current_bill > b.current_bill;
    return a.id > b.id;
}

void customers_bubble_sort(int sort_by)
{
    int i, j;
    Customer temp;

    for (i = 0; i < customer_count - 1; i++)
        for (j = 0; j < customer_count - 1 - i; j++)
            if (customer_is_after(customers[j], customers[j + 1], sort_by))
            {
                temp = customers[j];
                customers[j] = customers[j + 1];
                customers[j + 1] = temp;
            }
}

int ask_sort_type(void)
{
    printf("\nSort by:\n  1. Name\n  2. ID\n  3. Bill amount\n");
    return read_int("Choose: ", 1, 3);
}

void sort_customers(void)
{
    int sort_by = ask_sort_type();
    int i;

    if (customer_count == 0)
    {
        printf("\nThere are no customers to sort.\n");
        return;
    }
    customers_bubble_sort(sort_by);
    customer_print_header();
    for (i = 0; i < customer_count; i++)
        customer_print_row(i);
    customers_bubble_sort(SORT_BY_ID);
}


/* ======================================================================
   7. CART, BILL AND PAYMENT   (cart uses malloc/realloc growth)
   ====================================================================== */

int cart_ensure_room(void)
{
    CartItem *temp;

    if (cart_count < cart_room)
        return 1;
    cart_room += MAX_CART_GROW;
    temp = (CartItem *) realloc(cart, cart_room * sizeof(CartItem));
    if (temp == NULL)
        return 0;
    cart = temp;
    return 1;
}

int cart_find(int car_id)
{
    int i;

    for (i = 0; i < cart_count; i++)
        if (cart[i].car_id == car_id)
            return i;
    return -1;
}

int cart_add(int car_id, int quantity)
{
    int car_pos = find_car(car_id);
    int cart_pos = cart_find(car_id);
    int already = 0;

    if (cars[car_pos].stock == 0)
    {
        printf("Sorry, this car is out of stock.\n");
        return 0;
    }
    if (cart_pos != -1)
        already = cart[cart_pos].quantity;
    if (already + quantity > cars[car_pos].stock)
    {
        printf("Not enough stock. In stock: %d, already in cart: %d\n", cars[car_pos].stock, already);
        return 0;
    }
    if (cart_pos != -1)
        cart[cart_pos].quantity += quantity;
    else if (!cart_ensure_room())
    {
        printf("The cart is full.\n");
        return 0;
    }
    else
    {
        cart[cart_count].car_id = car_id;
        cart[cart_count].quantity = quantity;
        cart_count++;
    }
    printf("Cart updated.\n");
    return 1;
}

int cart_remove(int car_id)
{
    int i, cart_pos = cart_find(car_id);

    if (cart_pos == -1)
    {
        printf("That car is not in the cart.\n");
        return 0;
    }
    for (i = cart_pos; i < cart_count - 1; i++)
        cart[i] = cart[i + 1];
    cart_count--;
    printf("Car removed from cart.\n");
    return 1;
}

void cart_show(void)
{
    int i, car_pos;
    double price, total = 0;

    if (cart_count == 0)
    {
        printf("The cart is empty.\n");
        return;
    }
    printf("\n----------------- CART -----------------\n");
    printf("%-6s %-16s %-8s %-4s %-10s %s\n", "ID", "Model", "Variant", "Qty", "Price", "Total");
    for (i = 0; i < cart_count; i++)
    {
        car_pos = find_car(cart[i].car_id);
        price = price_get(cars[car_pos].category, cars[car_pos].company, cars[car_pos].variant);
        printf("%-6d %-16s %-8s %-4d %-10.2f %.2f\n", cars[car_pos].id, cars[car_pos].model,
               variant_names[cars[car_pos].variant - 1], cart[i].quantity, price, price * cart[i].quantity);
        total += price * cart[i].quantity;
    }
    printf("Cart total (before discounts): %.2f lakh\n", total);
}

void sale_add_car(void)
{
    int i, category, company, car_id, car_pos, quantity;
    int found = 0;

    show_list("Categories", category_names, CAR_CATEGORIES);
    category = read_int("Choose category (0 = back): ", 0, CAR_CATEGORIES);
    if (category == 0)
        return;
    show_list("Companies", company_names, CAR_COMPANIES);
    company = read_int("Choose company (0 = back): ", 0, CAR_COMPANIES);
    if (company == 0)
        return;

    car_print_header();
    for (i = 0; i < car_count; i++)
        if (cars[i].category == category && cars[i].company == company)
        {
            car_print_row(i);
            found = 1;
        }
    if (found == 0)
    {
        printf("No cars of this type in the showroom.\n");
        return;
    }
    car_id = read_int("Enter car ID to add (0 = back): ", 0, 999999);
    if (car_id == 0)
        return;
    car_pos = find_car(car_id);
    if (car_pos == -1 || cars[car_pos].category != category || cars[car_pos].company != company)
    {
        printf("That ID is not in the list above.\n");
        return;
    }
    quantity = read_int("Quantity: ", 1, 100);
    cart_add(car_id, quantity);
}

void print_receipt(int pos)
{
    if (customers[pos].current_bill <= 0)
    {
        printf("This customer has no bill.\n");
        return;
    }
    printf("\n=============== RECEIPT ===============\n");
    printf("Customer : %s (ID %d)\n", customers[pos].name, customers[pos].id);
    printf("Payment  : %s\n", payment_names[customers[pos].payment_type - 1]);
    printf("Bill     : %.2f lakh\n", customers[pos].current_bill);
    printf("Paid     : %.2f lakh\n", customers[pos].amount_paid);
    printf("Balance  : %.2f lakh\n", customers[pos].current_bill - customers[pos].amount_paid);
    printf("Status   : %s\n", status_names[customers[pos].payment_status - 1]);
    printf("=======================================\n");
}

void make_bill(int pos)
{
    static int bill_number = 0;         /* static: keeps its value between calls */
    int i, car_pos, total_quantity = 0;
    double price, subtotal = 0;
    double discount_percent = 0, discount_amount, after_discount;
    double charge_percent = 0, charge_amount, final_amount;
    int type = customers[pos].payment_type;

    bill_number++;
    printf("\n================ BILL No. %d ================\n", bill_number);
    printf("Customer : %s (ID %d)\n", customers[pos].name, customers[pos].id);
    printf("Payment  : %s\n", payment_names[type - 1]);
    printf("-------------------------------------------\n");
    for (i = 0; i < cart_count; i++)
    {
        car_pos = find_car(cart[i].car_id);
        price = price_get(cars[car_pos].category, cars[car_pos].company, cars[car_pos].variant);
        printf("%s %s (%s) x%d = %.2f\n", company_names[cars[car_pos].company - 1], cars[car_pos].model,
               variant_names[cars[car_pos].variant - 1], cart[i].quantity, price * cart[i].quantity);
        subtotal += price * cart[i].quantity;
        total_quantity += cart[i].quantity;
        cars[car_pos].stock -= cart[i].quantity;
    }
    printf("Subtotal                : %.2f\n", subtotal);

    if (customers[pos].age >= SENIOR_AGE)
    {
        discount_percent += DISCOUNT_SENIOR;
        printf("Senior citizen discount : %d%%\n", DISCOUNT_SENIOR);
    }
    if (customers[pos].previous_purchases >= 1)
    {
        discount_percent += DISCOUNT_LOYAL;
        printf("Loyal customer discount : %d%%\n", DISCOUNT_LOYAL);
    }
    if (total_quantity >= 2)
    {
        discount_percent += DISCOUNT_MULTI;
        printf("Multiple cars discount  : %d%%\n", DISCOUNT_MULTI);
    }
    if (type == PAY_CASH)
    {
        discount_percent += DISCOUNT_CASH;
        printf("Cash payment discount   : %d%%\n", DISCOUNT_CASH);
    }
    discount_amount = subtotal * discount_percent / 100;
    after_discount = subtotal - discount_amount;

    if (type == PAY_LOAN)
        charge_percent = CHARGE_LOAN;
    else if (type == PAY_LEASE)
        charge_percent = CHARGE_LEASE;
    charge_amount = after_discount * charge_percent / 100;
    final_amount = after_discount + charge_amount;

    printf("Total discount (%.0f%%)    : -%.2f\n", discount_percent, discount_amount);
    printf("Payment charge (%.0f%%)    : +%.2f\n", charge_percent, charge_amount);
    printf("FINAL AMOUNT (lakh)     : %.2f\n", final_amount);
    printf("===========================================\n");

    customers[pos].current_bill = final_amount;
    customers[pos].amount_paid = 0;
    customers[pos].payment_status = STATUS_UNPAID;
    cart_count = 0;
}

int pay_bill(int pos, double amount)
{
    double balance = customers[pos].current_bill - customers[pos].amount_paid;

    if (customers[pos].payment_status == STATUS_PAID)
    {
        printf("There is no unpaid bill for this customer.\n");
        return 0;
    }
    if (amount <= 0 || amount > balance + 0.001)
    {
        printf("Invalid amount. The balance is %.2f lakh.\n", balance);
        return 0;
    }
    customers[pos].amount_paid += amount;
    if (customers[pos].amount_paid >= customers[pos].current_bill - 0.001)
    {
        customers[pos].amount_paid = customers[pos].current_bill;
        customers[pos].payment_status = STATUS_PAID;
        customers[pos].previous_purchases++;
        printf("Payment complete. Thank you!\n");
    }
    else
    {
        customers[pos].payment_status = STATUS_PARTLY;
        printf("Partial payment received.\n");
    }
    print_receipt(pos);
    return 1;
}

void sale_start(int pos)
{
    int choice, payment_type;
    double balance;

    if (customers[pos].payment_status != STATUS_PAID)
    {
        printf("This customer still has an unpaid bill. Please pay it first.\n");
        return;
    }
    cart_count = 0;
    printf("\n========== NEW CAR SALE ==========\nCustomer: %s\n", customers[pos].name);
    do
    {
        printf("\n1. Add a car\n2. Remove a car\n3. Show cart\n4. Finish and make bill\n0. Cancel sale\n");
        choice = read_int("Choice: ", 0, 4);
        switch (choice)
        {
            case 1:
                sale_add_car();
                break;
            case 2:
                cart_show();
                if (cart_count > 0)
                    cart_remove(read_int("Car ID to remove: ", 1, 999999));
                break;
            case 3:
                cart_show();
                break;
            case 4:
                if (cart_count == 0)
                {
                    printf("Add at least one car first.\n");
                    break;
                }
                show_list("Payment type", payment_names, 3);
                payment_type = read_int("Choose payment type: ", 1, 3);
                customers[pos].payment_type = payment_type;
                make_bill(pos);
                if (read_int("Pay now? (1 = yes, 0 = later): ", 0, 1) == 1)
                {
                    balance = customers[pos].current_bill;
                    pay_bill(pos, read_double("Enter amount to pay: ", 0.01, balance));
                }
                else
                    printf("Bill saved as unpaid. It can be paid from the Customer Desk.\n");
                return;
            case 0:
                cart_count = 0;
                printf("Sale cancelled.\n");
                break;
        }
    } while (choice != 0);
}


/* ======================================================================
   8. PRIORITY QUEUE  (kept — not scheduling)
   ====================================================================== */

/* Shows customers in priority order (Urgent first). The customers array is
   not changed: only a list of positions is sorted. */
void queue_by_priority(void)
{
    int positions[MAX_CUSTOMERS];
    int i, j, temp;

    if (customer_count == 0)
    {
        printf("No customers registered.\n");
        return;
    }
    for (i = 0; i < customer_count; i++)
        positions[i] = i;
    for (i = 0; i < customer_count - 1; i++)
        for (j = 0; j < customer_count - 1 - i; j++)
            if (customers[positions[j]].type > customers[positions[j + 1]].type)
            {
                temp = positions[j];
                positions[j] = positions[j + 1];
                positions[j + 1] = temp;
            }
    printf("\n%-5s %-6s %-18s %s\n", "Rank", "ID", "Name", "Type");
    for (i = 0; i < customer_count; i++)
        printf("%-5d %-6d %-18s %s\n", i + 1, customers[positions[i]].id, customers[positions[i]].name,
               customer_type_names[customers[positions[i]].type - 1]);
}


/* ======================================================================
   9. SAVING AND LOADING  (text file handling)
   ====================================================================== */

/* File layout: counts line, customers, cars, price table */
int save_all(void)
{
    FILE *fp = fopen(DATA_FILE, "w");
    int i, row, col;

    if (fp == NULL)
        return 0;
    fprintf(fp, "%d %d %d %d\n", customer_count, next_customer_id, car_count, next_car_id);
    for (i = 0; i < customer_count; i++)
        fprintf(fp, "%d %s %s %d %d %d %f %f %d %d %s\n", customers[i].id, customers[i].name,
                customers[i].phone, customers[i].age, customers[i].type, customers[i].previous_purchases,
                customers[i].current_bill, customers[i].amount_paid, customers[i].payment_type,
                customers[i].payment_status, customers[i].registration_date);
    for (i = 0; i < car_count; i++)
        fprintf(fp, "%d %d %d %s %d %d\n", cars[i].id, cars[i].category, cars[i].company,
                cars[i].model, cars[i].variant, cars[i].stock);
    for (row = 0; row < CAR_CATEGORIES; row++)
    {
        for (col = 0; col < CAR_COMPANIES; col++)
            fprintf(fp, "%f ", price_table[row][col]);
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 1;
}

int load_all(void)
{
    FILE *fp = fopen(DATA_FILE, "r");
    int i, row, col;

    if (fp == NULL)
        return 0;
    if (fscanf(fp, "%d %d %d %d", &customer_count, &next_customer_id, &car_count, &next_car_id) != 4 ||
        customer_count < 0 || customer_count > MAX_CUSTOMERS || car_count < 0 || car_count > MAX_CARS)
    {
        fclose(fp);
        customer_count = 0;
        car_count = 0;
        return 0;
    }
    /* grow the dynamic customer array as needed */
    while (customer_room < customer_count)
    {
        Customer *temp = (Customer *) realloc(customers,
                            (customer_room + CUST_GROW_ROOM) * sizeof(Customer));
        if (temp == NULL)
        {
            fclose(fp);
            customer_count = 0;
            car_count = 0;
            return 0;
        }
        customers = temp;
        customer_room += CUST_GROW_ROOM;
    }
    for (i = 0; i < customer_count; i++)
        if (fscanf(fp, "%d %29s %14s %d %d %d %lf %lf %d %d %11s", &customers[i].id, customers[i].name,
                   customers[i].phone, &customers[i].age, &customers[i].type,
                   &customers[i].previous_purchases, &customers[i].current_bill, &customers[i].amount_paid,
                   &customers[i].payment_type, &customers[i].payment_status,
                   customers[i].registration_date) != 11)
        {
            fclose(fp);
            customer_count = 0;
            car_count = 0;
            return 0;
        }
    for (i = 0; i < car_count; i++)
        if (fscanf(fp, "%d %d %d %29s %d %d", &cars[i].id, &cars[i].category, &cars[i].company,
                   cars[i].model, &cars[i].variant, &cars[i].stock) != 6)
        {
            fclose(fp);
            customer_count = 0;
            car_count = 0;
            return 0;
        }
    for (row = 0; row < CAR_CATEGORIES; row++)
        for (col = 0; col < CAR_COMPANIES; col++)
            fscanf(fp, "%lf", &price_table[row][col]);
    fclose(fp);
    return 1;
}

void add_sample_car(int category, int company, char model[], int variant, int stock)
{
    cars[car_count].id = next_car_id;
    cars[car_count].category = category;
    cars[car_count].company = company;
    strcpy(cars[car_count].model, model);
    cars[car_count].variant = variant;
    cars[car_count].stock = stock;
    car_count++;
    next_car_id++;
}

void add_sample_customer(char name[], char phone[], int age, int type, int purchases)
{
    Customer c;

    customer_ensure_room();
    c.id = next_customer_id;
    strcpy(c.name, name);
    strcpy(c.phone, phone);
    c.age = age;
    c.type = type;
    c.previous_purchases = purchases;
    c.current_bill = 0;
    c.amount_paid = 0;
    c.payment_type = PAY_CASH;
    c.payment_status = STATUS_PAID;
    strcpy(c.registration_date, today_date);
    customers[customer_count] = c;
    customer_count++;
    next_customer_id++;
}

void load_sample_data(void)
{
    add_sample_car(1, 3, "Swift", 2, 5);
    add_sample_car(1, 1, "Yaris", 1, 4);
    add_sample_car(2, 2, "Civic", 3, 3);
    add_sample_car(2, 1, "Corolla", 2, 6);
    add_sample_car(3, 4, "Tucson", 3, 2);
    add_sample_car(3, 5, "Sportage", 2, 4);
    add_sample_car(3, 1, "Fortuner", 3, 2);
    add_sample_car(4, 2, "HR-V", 1, 3);
    add_sample_car(7, 1, "Hilux", 2, 5);
    add_sample_car(8, 3, "Every", 1, 6);
    add_sample_car(9, 4, "Ioniq5", 3, 2);
    add_sample_car(10, 1, "Land_Cruiser", 3, 1);
    add_sample_customer("Ahmed_Khan", "03001234567", 35, 5, 0);
    add_sample_customer("Sara_Malik", "03111234567", 62, 2, 1);
    add_sample_customer("Bilal_Raza", "03211234567", 28, 3, 0);
}


/* ======================================================================
   10. REPORTS
   ====================================================================== */

void report_daily_count(void)
{
    int i, count = 0;

    printf("\n===== CUSTOMERS REGISTERED TODAY (%s) =====\n", today_date);
    for (i = 0; i < customer_count; i++)
    {
        if (strcmp(customers[i].registration_date, today_date) != 0)
            continue;
        count++;
        printf("  %d. ID %d - %s\n", count, customers[i].id, customers[i].name);
    }
    printf("Total registered today: %d\n", count);
}

void report_unpaid_bills(void)
{
    int i, count = 0;
    double remaining, total = 0;

    printf("\n===== UNPAID BILLS =====\n");
    printf("%-6s %-18s %-10s %-10s %-10s %s\n", "ID", "Name", "Bill", "Paid", "Remaining", "Status");
    for (i = 0; i < customer_count; i++)
    {
        if (customers[i].payment_status == STATUS_PAID)
            continue;
        remaining = customers[i].current_bill - customers[i].amount_paid;
        total += remaining;
        count++;
        printf("%-6d %-18s %-10.2f %-10.2f %-10.2f %s\n", customers[i].id, customers[i].name,
               customers[i].current_bill, customers[i].amount_paid, remaining,
               status_names[customers[i].payment_status - 1]);
    }
    if (count == 0)
        printf("No unpaid bills. Everyone has paid.\n");
    else
        printf("Customers with unpaid bills: %d, total pending: %.2f lakh\n", count, total);
}


/* ======================================================================
   11. MENUS
   ====================================================================== */

int date_is_valid(char date[])
{
    int i;

    if (strlen(date) != 10)
        return 0;
    for (i = 0; i < 10; i++)
    {
        if (i == 2 || i == 5)
        {
            if (date[i] != '-')
                return 0;
        }
        else if (!isdigit((unsigned char)date[i]))
            return 0;
    }
    return 1;
}

int admin_login(void)
{
    int attempt;

    if (admin_locked)
    {
        printf("\nAdmin menu is LOCKED (too many wrong PINs). Restart the program.\n");
        return 0;
    }
    for (attempt = 1; attempt <= PIN_ATTEMPTS; attempt++)
    {
        if (read_int("\nEnter admin PIN: ", 0, 999999) == ADMIN_PIN)
        {
            printf("PIN correct. Welcome, Admin!\n");
            return 1;
        }
        printf("Wrong PIN. Attempts left: %d\n", PIN_ATTEMPTS - attempt);
    }
    admin_locked = 1;
    printf("\nToo many wrong attempts. Admin menu is now LOCKED.\n");
    return 0;
}

void admin_menu(void)
{
    int choice, id, pos;

    do
    {
        print_title("ADMIN MENU");
        printf(" 1. Add a car\n");
        printf(" 2. Update a car (details, price, stock)\n");
        printf(" 3. Register a customer\n");
        printf(" 4. Update a customer\n");
        printf(" 5. Delete a customer\n");
        printf(" 6. List all customers\n");
        printf(" 7. Report: customers registered today\n");
        printf(" 8. Report: unpaid bills\n");
        printf(" 9. Sort customers\n");
        printf("10. Search customer by ID (recursive binary search)\n");
        printf("11. Search customer by name\n");
        printf("12. Search car by model\n");
        printf(" 0. Back to main menu\n");
        print_rule('=', 60);
        choice = read_int("Enter your choice: ", 0, 12);
        switch (choice)
        {
            case 1:  car_add(); break;
            case 2:  car_update(read_int("Enter car ID: ", 1, 999999)); break;
            case 3:  customer_register(); break;
            case 4:  customer_update(read_int("Enter customer ID: ", 1, 999999)); break;
            case 5:  customer_delete(read_int("Enter customer ID: ", 1, 999999)); break;
            case 6:  customer_list_all(); break;
            case 7:  report_daily_count(); break;
            case 8:  report_unpaid_bills(); break;
            case 9:  sort_customers(); break;
            case 10:
                id = read_int("Enter customer ID to search: ", 1, 999999);
                pos = search_customer_by_id(id);
                if (pos < 0)
                    printf("  Customer ID %d was not found.\n", id);
                else
                {
                    customer_print_header();
                    customer_print_row(pos);
                }
                break;
            case 11: search_customer_by_name(); break;
            case 12: search_car_by_model(); break;
        }
        if (choice != 0)
            pause_screen();
    } while (choice != 0);
}

void executive_menu(void)
{
    int choice, pos;

    do
    {
        print_title("SALES EXECUTIVE MENU");
        printf("1. Show customers by priority (Urgent first)\n");
        printf("2. Create a car sale\n");
        printf("0. Back to main menu\n");
        print_rule('=', 60);
        choice = read_int("Enter your choice: ", 0, 2);
        switch (choice)
        {
            case 1:
                queue_by_priority();
                break;
            case 2:
                pos = ask_customer_position();
                if (pos >= 0)
                    sale_start(pos);
                break;
        }
        if (choice != 0)
            pause_screen();
    } while (choice != 0);
}

void desk_menu(void)
{
    int choice, pos;
    double remaining;

    do
    {
        print_title("CUSTOMER DESK MENU");
        printf("1. Register a new customer\n");
        printf("2. Browse cars by category\n");
        printf("3. Search car by model\n");
        printf("4. Search customer by name\n");
        printf("5. View my bill\n");
        printf("6. Pay my bill\n");
        printf("7. Print receipt\n");
        printf("0. Back to main menu\n");
        print_rule('=', 60);
        choice = read_int("Enter your choice: ", 0, 7);
        switch (choice)
        {
            case 1:
                customer_register();
                break;
            case 2:
                show_list("Car categories (0 = all)", category_names, CAR_CATEGORIES);
                car_browse(read_int("Choose category: ", 0, CAR_CATEGORIES));
                break;
            case 3:
                search_car_by_model();
                break;
            case 4:
                search_customer_by_name();
                break;
            case 5:
            case 7:
                pos = ask_customer_position();
                if (pos >= 0)
                    print_receipt(pos);
                break;
            case 6:
                pos = ask_customer_position();
                if (pos < 0)
                    break;
                remaining = customers[pos].current_bill - customers[pos].amount_paid;
                if (remaining <= 0.001)
                {
                    printf("  Nothing to pay for this customer.\n");
                    break;
                }
                printf("Remaining to pay: %.2f lakh\n", remaining);
                pay_bill(pos, read_double("Enter amount to pay: ", 0.01, remaining));
                break;
        }
        if (choice != 0)
            pause_screen();
    } while (choice != 0);
}

void main_menu(void)
{
    int choice;

    do
    {
        print_title("SMART CAR SHOWROOM MANAGEMENT");
        printf("1. Admin\n");
        printf("2. Sales Executive\n");
        printf("3. Customer Desk\n");
        printf("0. Exit\n");
        print_rule('=', 60);
        choice = read_int("Enter your choice: ", 0, 3);
        switch (choice)
        {
            case 1:
                if (admin_login())
                    admin_menu();
                break;
            case 2: executive_menu(); break;
            case 3: desk_menu(); break;
            case 0: printf("\nSaving data and exiting...\n"); break;
        }
    } while (choice != 0);
}

int main(void)
{
    /* allocate the initial customer list (10 slots) and cart (5 slots) */
    customers = (Customer *) malloc(CUST_INIT_ROOM * sizeof(Customer));
    cart      = (CartItem *) malloc(MAX_CART_INIT * sizeof(CartItem));
    if (customers == NULL || cart == NULL)
    {
        printf("Not enough memory.\n");
        return 1;
    }
    customer_room = CUST_INIT_ROOM;
    cart_room     = MAX_CART_INIT;

    do
    {
        if (read_text("Enter today's date (DD-MM-YYYY): ", today_date, DATE_LENGTH) == 0)
        {
            free(customers);
            free(cart);
            return 0;
        }
        if (!date_is_valid(today_date))
            printf("  Wrong format. Example: 31-10-2026\n");
    } while (!date_is_valid(today_date));

    if (!load_all())
    {
        printf("No saved data found. Loading sample data...\n");
        load_sample_data();
    }
    main_menu();

    if (save_all())
        printf("Data saved. Goodbye!\n");
    else
        printf("Warning: data could not be saved.\n");
    free(customers);
    free(cart);
    return 0;
}