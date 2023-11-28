#!/usr/bin/python3.12

'''
A python implementation of an LL1 parser I wrote for a class.

Uses a dictionary as the parse table.

Sample parse table and input string is given. I've made some
changes since the original program to make it more user-friendly.

(ghost-1608 26/11/23)
'''

# Some definition to make the grammar more flexible
EPSILON = '#'

# Defining hash table type
type htable = dict[str, dict[str, str]]

# Wrapper for looking nice (it's a function!)
def isnonterminal(x: str, table: htable) -> bool:
    '''
    Checks whether given char is terminal or not (using parse table)
    '''
    return x in table

# Brains :)
def ll1(inp: str, start_symbol: str, table: htable) -> bool:
    '''
    LL1 implementation
    Needs input string, starting grammar symbol, and parse table
    Returns bool value on acceptance or not
    '''

    # stack for use
    stack: list[str] = []

    # Prepare stack and input string
    stack.append('$')
    stack.append(start_symbol)
    inp += '$'

    # Main loop
    i: int = 0
    while len(stack) > 1:
        chr: str = inp[i]
        symbol: str = stack.pop()

        if isnonterminal(symbol, table):
            # Replace stack symbol using parse table
            new_symbols: str = table[symbol][chr]
            stack.extend(list(new_symbols[::-1]))
        elif chr == EPSILON or chr == symbol:
            # Move to next input string
            i += 1

    # Final acceptance condition
    if stack == ['$'] and inp[i - 1] == '$':
        stack.pop()
        return True

    return False

if __name__ == '__main__':
    # Sample parse table (ONLY FOR PRINTING)
    parse_table: str = '''\
+-------------------------------------+
|   ||  (  | i  | $ |  +  |  *  |  )  |
|=====================================|
| S ||  E$ | E$ |   |     |     |     |
| E ||  TF | TF |   |     |     |     |
| F ||     |    | # | +TF |     |  #  |
| T ||  GU | GU |   |     |     |     |
| U ||     |    | # |  #  | *GU |  #  |
| G || (E) | i  |   |     |     |     |
+-------------------------------------+
    '''

    # Sample parse table
    table: htable = {
        'S': {'(': 'E$', 'i': 'E$'},
        'E': {'(': 'TF', 'i': 'TF'},
        'F': {'$': '#', '+': '+TF', ')': '#'},
        'T': {'(': 'GU', 'i': 'GU'},
        'U': {'$': '#', '+': '#', '*': '*GU', ')': '#'},
        'G': {'(': '(E)', 'i': 'i'}
        }

    # Some user-friendlyness
    print('Parse Table')
    print('===========')

    print(parse_table)

    print('Input String: "i+i"\n')

    # Result
    print('Result: ', end='')
    if ll1('i+i', 'S', table):
        print('Accepted!')
    else:
        print('Rejected!')
