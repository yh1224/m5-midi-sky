#if !defined(APP_NOTES_H)
#define APP_NOTES_H

class Notes
{
public:
    Notes() = default;

    explicit Notes(const unsigned long timestamps[15])
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

    bool operator!=(const Notes& other) const
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

// Stateful filter to prevent old notes from reappearing
class NotesFilter
{
public:
    Notes latest(const Notes& notes, const int num)
    {
        unsigned long newTimestamps[15] = {0};
        bool used[15] = {false};

        for (int n = 0; n < num; n++) {
            unsigned long maxVal = 0;
            int maxIdx = -1;
            for (int i = 0; i < 15; i++) {
                const unsigned long timestamp = notes.get(i);
                // Only consider notes that are currently pressed AND newer than cutoff
                if (!used[i] && timestamp > 0 && timestamp > cutoffThreshold && timestamp > maxVal) {
                    maxVal = timestamp;
                    maxIdx = i;
                }
            }
            if (maxIdx == -1) {
                break;
            }
            newTimestamps[maxIdx] = maxVal;
            used[maxIdx] = true;
        }

        // Update cutoff threshold: find the oldest unselected note that's newer than current cutoff
        unsigned long maxUnselectedTimestamp = cutoffThreshold;
        for (int i = 0; i < 15; i++) {
            const unsigned long timestamp = notes.get(i);
            if (!used[i] && timestamp > cutoffThreshold && timestamp > maxUnselectedTimestamp) {
                maxUnselectedTimestamp = timestamp;
            }
        }
        cutoffThreshold = maxUnselectedTimestamp;

        return Notes(newTimestamps);
    }

private:
    unsigned long cutoffThreshold = 0;
};

#endif // !defined(APP_NOTES_H)
