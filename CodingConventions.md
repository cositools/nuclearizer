# Coding conventions

## New classes

For new classes, copy and modify an existing one, or use the MModuleTemplate class as tempplate.

## Naming

### Classes

* Due to historic reasons, all classes start with M.
* Modules start with MModule
* Option GUI's start with MGUIOptions
* Expo (= data display) GUI's start with MGUIExpo

### Class functions

The naming follows the "upper camel case" convention, e.g.,
```
MVector, Clear, IsNull, SetMagThetaPhi
```

### Member variables start with

Member variables start with "m_", and the remainder also follow the "upper camel case" convention, e.g.,
```
m_X, m_DataPoint, m_IsNonZero
```

### Variables in functions

Variables in functions *should* follow the "upper camel case" convention, e.g.,
```
X, DataPoint, IsNonZero
```

## Comments

Use doxygen-style comments (//!) for classes, member functions, and variables, including a brief description of the method's functionality.
```
//! Standard constructor giving x, y, z component of the data
MVector(double X = 0.0, double Y = 0.0, double Z = 0.0);


//! Flag indicating if the vector is zero
bool m_IsZero;
````

Use single-line comments (//) to explain logic within methods.


## Formatting


## Whitespace Conventions

### 1. **General Whitespace Guidelines**
- **Consistency**: Be consistent in the use of whitespace across the codebase. This promotes readability and makes the code easier to follow.
- **Spaces Around Operators**: Always place a space around binary operators (e.g., `+`, `-`, `=`, `==`, `&&`, etc.).
  ```cpp
  m_X += W.m_X;  // Correct
  m_X+=W.m_X;    // Incorrect
  ```
- **Function Parameters++: There should be a space after a comma separating parameters in function declarations and calls.
  ```cpp
  MVector(double X, double Y, double Z);  // Correct
  MVector(double X,double Y,double Z);    // Incorrect
  ```

### 2. **Indentation**

- **Indentation Level**: Use 2 spaces for indentation (no tabs). This applies to all indented blocks of code such as loops, conditionals, and class methods.
- **Alignment**: Ensure that all lines within the same block are aligned, especially for multiple function arguments or complex expressions. For instance:
  ```cpp
    m_X = V.m_X;
    m_Y = V.m_Y;
    m_Z = V.m_Z;
  ```

### 3. **Blank Lines Between Functions and Code Blocks**

- **Between Functions**: Insert the following seperating blockbetween functions to separate them visually and enhance readability.
  ```cpp
void SetX(double x)
{
  m_X = x;
}


////////////////////////////////////////////////////////////////////////////////


void SetY(double y)
{
  m_Y = y;
}
  ```

- **Within Functions**: Use blank lines to separate logical blocks of code within a function. For example, separate initialization, computation, and return statements:
  ```cpp
    void SetMagThetaPhi(double mag, double theta, double phi)
    {
      double r = Mag();
      double t = Theta();

      m_X = r*sin(t)*cos(p);
      m_Y = r*sin(t)*sin(p);
      m_Z = r*cos(t);
    }
  ```

### 4. **Spaces Around Braces inside functions**

- **Opening Brace**: The opening brace { should be placed at the end of the line for function definitions, conditionals, loops, and class definitions - the exception are meber functions, where it is placed on a single new line.
  ```cpp
if (m_X == 0) {  // Correct
  // Do something
}
if (m_X == 0) // Incorrect
{
  // Do something
}
  ```

- **Closing Brace**: The closing brace } should be on its own line, aligned with the line where the corresponding opening brace appeared.
  ```cpp
    void SomeFunction() {
      // Code logic here
    }
  ```

### 5. **Spaces After Keywords**

- **Space After Control Flow Keywords**: Always add a space after control flow keywords like if, else, for, while, switch, try, etc.
  ```cpp
if (x > y) {  // Correct
  // do something
}

for (int i = 0; i < n; i++) {  // Correct
  // loop body
}
   ```
- **No Space Before Parentheses in Control Flow**: There should be no space between the keyword and the opening parenthesis.
  ```cpp
    if (x == y) {  // Correct
      // do something
    }

    while (x < 10) {  // Correct
      // loop body
    }
  ```


### 6. **Whitespace in Expressions**

- **Space Around Operators**: Always add a space before and after most binary operators (=, +, -, &&, etc.). The exce[tion are * and / in math expresions, to visualize the order of operations.
  ```cpp
double x = 5 + 3*2;    // Correct
double x = 5+3*2;      // Incorrect
double x = 5 + 3 * 2;  // Incorrect
  ```

- **Function Calls**: No spaces between the function name and the opening parenthesis. Add spaces between arguments if needed, but avoid excessive whitespace.
  ```cpp
SomeFunction(x, y);    // Correct
SomeFunction (x, y);   // Incorrect
  ```

- **Unary Operators**: Do not add spaces between unary operators (e.g., ++, --, !, -).
  ```cpp
    m_X++;   // Correct
    ++m_X;   // Correct
    --m_X;   // Correct
    m_X ++;   // Incorrect
  ```

- **cout etc.**: There are no white spaces before and after <<, >>
  ```cpp
    cout<<"Var: "<<V<<endl;        // Correct
    cout << "Var: " << V << endl;  // Incorrect
  ```

### 7. **Trailing Whitespace**

- **Avoid Trailing Whitespace**: Do not leave trailing spaces at the end of lines. Trailing spaces can cause version control diffs to become unnecessarily cluttered.






