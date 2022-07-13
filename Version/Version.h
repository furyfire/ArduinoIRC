class Version {
public:
    Version(unsigned int, unsigned int, unsigned int);
private:
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
};

#define VERSION_MACRO(major,minor,patch) 