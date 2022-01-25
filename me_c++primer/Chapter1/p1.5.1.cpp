#include "Sales_item.h"

int Item_output()
{
    Sales_item book;
    if (!(std::cin >> book)){
        return 0;
    }
    std::cout << book << std::endl;
    return 1;
}

int main()
{
    while(Item_output());
    return 0;
}