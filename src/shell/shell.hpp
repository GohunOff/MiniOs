#ifndef MINIOS_SHELL_HPP
#define MINIOS_SHELL_HPP

class Shell
{
public:
    void execute(const char* command);
    void printHelp() const;

private:
    using Handler = void (*)(Shell&, const char*);

    struct Command
    {
        const char* name;
        const char* description;
        Handler handler;
    };

    static const Command commands[];
};

#endif // MINIOS_SHELL_HPP