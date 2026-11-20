# Smart Car Showroom Management System

A console (text based) program written in **C** that manages a car showroom: cars, customers, sales, bills and payments.
This is our **Programming Fundamentals (CT-175) CCA project**.

### Group Members

| Student Name | Roll Number |
| :--- | :--- |
| Faisal Irfan | CT-26247 |
| Sarim Ali | CT-26245 |
| Abdul Rehman | CT-26231 |
| Shahzaib Rehman | CT-26233 |

### Project Details

| Field | Details |
| :--- | :--- |
| **Course** | Programming Fundamentals (CT-175) - CCA Project |
| **Department** | Computer Science & IT Department |
| **Instructor** | Maham Ashraf |
| **Language** | C (standard C, one source file) |


## 1. Project description

The program is built for a car showroom with three kinds of users:

- **Admin** (needs a PIN) - manages cars and customers, sees reports, sorts and searches.
- **Sales Executive** - sees the priority list of customers and creates car sales (cart, bill, payment).
- **Customer Desk** - customers can register, browse cars, view their bill, pay and print a receipt.

The showroom sells cars from 5 companies (Toyota, Honda, Suzuki, Hyundai, Kia) in 10 categories
(Hatchback, Sedan, SUV, Crossover, Coupe, Convertible, Pickup Truck, Van/MPV, Electric, Luxury).
Every car has 3 variants (Base, Mid, Top). Prices are in **lakh**. All data is saved in a text file when the
program exits and is loaded again when it starts.

## 2. Features

**Admin menu**
1. Add a car / update a car (model, variant, stock, base price)
2. Register, update and delete customers (a customer with an unpaid bill cannot be deleted)
3. List all customers
4. Reports: customers registered today, unpaid bills (with total pending amount)
5. Sort customers by name, ID or bill amount (bubble sort)
6. Search a customer by ID (recursive binary search) or by name, search a car by model

**Sales Executive menu**
1. Show customers by priority: Urgent Delivery, VIP, Corporate, Pre-booked, Normal
2. Create a car sale: add car, remove car, show cart, finish and make the bill, pay now or later

**Customer Desk menu**
1. Register a new customer
2. Browse cars by category, search a car by model, search a customer by name
3. View bill, pay bill (full or part payment), print receipt

**Other**
- Input validation for every question (numbers, ranges, empty text, date format, PIN)
- Admin PIN with 3 attempts, then the admin menu is locked until the program is restarted
- Stock check before a car is added to the cart
- Data saved to `showroom_data.txt` and loaded on the next start (sample data is used if no file exists)

## 3. Billing rules

| Rule | Value |
|---|---|
| Car price | base price of (category, company) x variant factor (Base 1.00, Mid 1.15, Top 1.30) |
| Senior citizen discount (age 60 or more) | 5% |
| Loyal customer discount (at least 1 earlier purchase) | 3% |
| Multiple cars discount (2 or more cars in one sale) | 4% |
| Cash payment discount | 2% |
| Bank loan charge (on the discounted amount) | 10% |
| Lease charge (on the discounted amount) | 6% |

All applicable discounts are added together. Payment status can be **Unpaid**, **Partly Paid** or **Paid**.

Example: Sara (62 years, one earlier purchase) buys a Toyota Yaris (Base, 38.00 lakh) and pays cash.
Discount = 5% + 3% + 2% = 10% = 3.80. Final bill = **34.20 lakh**.

## 4. Programming Fundamentals concepts used

| Concept | Where it is used |
|---|---|
| Variables, data types, constants (`#define`) | `int`, `double`, `char`; `MAX_CARS`, `ADMIN_PIN`, `DISCOUNT_SENIOR`, ... |
| Operators, `if / else`, `switch` | `make_bill`, `pay_bill`, all menus |
| Loops (`for`, `while`, `do-while`) | menus, `read_int`, lists, searching, sorting |
| Functions (parameters, return values) | about 60 small functions, e.g. `price_get`, `find_car`, `cart_add` |
| Recursion | `search_customer_by_id_rec` (binary search) |
| Arrays (1D and 2D) | `cars[]`, `price_table[10][5]`, `category_names[10][30]` |
| Strings | `strcpy`, `strcmp`, `strlen`, own `same_text`, `read_text` |
| Structures | `Customer`, `Car`, `CartItem`, array of structures |
| Pointers | `Customer *customers`, `CartItem *cart`, `FILE *fp` |
| Dynamic memory | `malloc`, `realloc`, `free` for the customer list and the cart |
| File handling | `save_all` (`fopen`, `fprintf`) and `load_all` (`fscanf`) |
| Searching and sorting | linear search, recursive binary search, bubble sort |
| Global and `static` variables | global arrays and counters, `static int bill_number` |
| Input validation | `read_int`, `read_double`, `read_text`, `date_is_valid` |

## 5. Project structure

```
PF_CCA_Project_Group14/
|-- Source_Code/
|   `-- main.c                  (complete source code)
|-- Project_Report.pdf          (project report)
|-- Flowchart.pdf               (flowcharts)
|-- README.md                   (this file)
`-- Screenshots/                (output1.png ... output26.png)
```

`showroom_data.txt` is created automatically in the folder where the program runs, when the program exits.

## 6. How to compile and run

You need a C compiler such as GCC (MinGW on Windows, or the GCC that comes with Code::Blocks / Dev-C++).

```bash
gcc Source_Code/main.c -o project
./project             # on Windows: project.exe
```

1. Type today's date in the format `DD-MM-YYYY` (example: `20-11-2026`).
2. Choose a menu option by typing its number and pressing Enter.
3. The admin PIN is **1234**.
4. Choose `0` in the main menu to save the data and exit.

## 7. Sample data (loaded when there is no saved file)

**Cars**

| ID | Category | Company | Model | Variant | Price (lakh) | Stock |
|---|---|---|---|---|---|---|
| 2001 | Hatchback | Suzuki | Swift | Mid | 28.75 | 5 |
| 2002 | Hatchback | Toyota | Yaris | Base | 38.00 | 4 |
| 2003 | Sedan | Honda | Civic | Top | 67.60 | 3 |
| 2004 | Sedan | Toyota | Corolla | Mid | 63.25 | 6 |
| 2005 | SUV | Hyundai | Tucson | Top | 104.00 | 2 |
| 2006 | SUV | Kia | Sportage | Mid | 97.75 | 4 |
| 2007 | SUV | Toyota | Fortuner | Top | 123.50 | 2 |
| 2008 | Crossover | Honda | HR-V | Base | 68.00 | 3 |
| 2009 | Pickup Truck | Toyota | Hilux | Mid | 97.75 | 5 |
| 2010 | Van/MPV | Suzuki | Every | Base | 42.00 | 6 |
| 2011 | Electric | Hyundai | Ioniq5 | Top | 123.50 | 2 |
| 2012 | Luxury | Toyota | Land_Cruiser | Top | 286.00 | 1 |

**Customers**

| ID | Name | Age | Type | Earlier purchases |
|---|---|---|---|---|
| 1001 | Ahmed_Khan | 35 | Normal | 0 |
| 1002 | Sara_Malik | 62 | VIP | 1 |
| 1003 | Bilal_Raza | 28 | Corporate | 0 |

Note: names are stored with `_` instead of spaces (the program changes spaces to `_` automatically).

## 8. Sample run

```
Enter today's date (DD-MM-YYYY): 20-11-2026
No saved data found. Loading sample data...
               SMART CAR SHOWROOM MANAGEMENT
1. Admin
2. Sales Executive
3. Customer Desk
0. Exit
Enter your choice: abc
  Invalid input. Please type a whole number.
Enter your choice: 9
  Number must be between 0 and 3.
```

Sale example (Sales Executive > Create a car sale > customer 1002 > Yaris > Cash):

```
================ BILL No. 1 ================
Customer : Sara_Malik (ID 1002)
Payment  : Cash
Toyota Yaris (Base) x1 = 38.00
Subtotal                : 38.00
Senior citizen discount : 5%
Loyal customer discount : 3%
Cash payment discount   : 2%
Total discount (10%)    : -3.80
Payment charge (0%)    : +0.00
FINAL AMOUNT (lakh)     : 34.20
```

## 9. Testing

20 test cases were run (date check, menu validation, PIN lock, registration validation, binary search,
adding cars, stock check, discounts, cash / loan / part payments, delete rules, sorting, priority queue,
save and load). One test (exact-balance payment) failed at first because of a floating-point rounding bug; it was fixed and passed on re-run. The full table with inputs, expected and actual output is in **Project_Report.pdf**.

## 10. Screenshots

26 screenshots of the working program are in the `Screenshots/` folder (`output1.png` to `output26.png`). Each one is shown with its caption in Section 8 of `Project_Report.pdf`. Main ones:

| File | What it shows |
|---|---|
| `Screenshots/output1.png` | Date check and main menu |
| `Screenshots/output4.png` | Admin menu after PIN |
| `Screenshots/output6.png` | List of customers |
| `Screenshots/output9.png` | Cash bill with discounts |
| `Screenshots/output17.png` | Payment complete |
| `Screenshots/output13.png` | Customers sorted by bill |

## 11. Documents

- `Project_Report.pdf` - introduction, objectives, functionalities, PF concepts, program logic, test cases, evidence, summary
- `Flowchart.pdf` - overall flow, the three menus, cart and billing

## 12. Authors

Faisal Irfan, Sarim Ali, Abdul Rehman and Shahzaib Rehman - Computer Science students
