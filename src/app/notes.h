#if !defined(APP_NOTES_H)
#define APP_NOTES_H

class Notes15
{
public:
    Notes15() = default;

    explicit Notes15(const unsigned long timestamps[15])
    {
        for (int i = 0; i < 15; i++) {
            this->timestamps[i] = timestamps[i];
        }
    }

    unsigned long get(const int index) const
    {
        if (index >= 0 && index < 15) {
            return timestamps[index];
        }
        return 0;
    }

    bool operator!=(const Notes15& other) const
    {
        for (int i = 0; i < 15; i++) {
            if (timestamps[i] != other.timestamps[i]) {
                return true;
            }
        }
        return false;
    }

private:
    unsigned long timestamps[15]{};
};

#endif // !defined(APP_NOTES_H)
