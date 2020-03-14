#include "decl.h"

int stoi_safe(const std::string &str, int *p_value, std::size_t *pos, int base)
{
    // wrapping std::stoi because it may throw an exception

    try
    {
        *p_value = std::stoi(str, pos, base);
        return 0;
    }

    catch (const std::invalid_argument &ia)
    {
        //std::cerr << "Invalid argument: " << ia.what() << std::endl;
        return -1;
    }

    catch (const std::out_of_range &oor)
    {
        //std::cerr << "Out of Range error: " << oor.what() << std::endl;
        return -2;
    }

    catch (const std::exception &e)
    {
        //std::cerr << "Undefined error: " << e.what() << std::endl;
        return -3;
    }
}

int stoi_safe(const std::string &str, size_t *p_value, std::size_t *pos, int base)
{
    // wrapping std::stoul because it may throw an exception

    try
    {
        *p_value = std::stoul(str, pos, base);
        return 0;
    }

    catch (const std::invalid_argument &ia)
    {
        //std::cerr << "Invalid argument: " << ia.what() << std::endl;
        return -1;
    }

    catch (const std::out_of_range &oor)
    {
        //std::cerr << "Out of Range error: " << oor.what() << std::endl;
        return -2;
    }

    catch (const std::exception &e)
    {
        //std::cerr << "Undefined error: " << e.what() << std::endl;
        return -3;
    }
}

// accept literal arguments
int stoi_safe(const std::string &&str, size_t *p_value, std::size_t *pos, int base)
{
    // wrapping std::stoul because it may throw an exception

    try
    {
        *p_value = std::stoul(str, pos, base);
        return 0;
    }

    catch (const std::invalid_argument &ia)
    {
        //std::cerr << "Invalid argument: " << ia.what() << std::endl;
        return -1;
    }

    catch (const std::out_of_range &oor)
    {
        //std::cerr << "Out of Range error: " << oor.what() << std::endl;
        return -2;
    }

    catch (const std::exception &e)
    {
        //std::cerr << "Undefined error: " << e.what() << std::endl;
        return -3;
    }
}