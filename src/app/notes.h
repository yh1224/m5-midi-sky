#if !defined(APP_NOTES_H)
#define APP_NOTES_H

class Notes
{
public:
    explicit Notes(const int numNotes)
        : numNotes(numNotes)
          , timestamps(std::make_unique<unsigned long[]>(numNotes))
    {
        for (int i = 0; i < numNotes; i++) {
            this->timestamps[i] = 0;
        }
    }

    explicit Notes(const int numNotes, const unsigned long timestamps[])
        : numNotes(numNotes)
          , timestamps(std::make_unique<unsigned long[]>(numNotes))
    {
        for (int i = 0; i < numNotes; i++) {
            this->timestamps[i] = timestamps[i];
        }
    }

    // Copy constructor
    Notes(const Notes& other)
        : numNotes(other.numNotes)
          , timestamps(std::make_unique<unsigned long[]>(other.numNotes))
    {
        for (int i = 0; i < numNotes; i++) {
            timestamps[i] = other.timestamps[i];
        }
    }

    // Copy assignment operator
    Notes& operator=(const Notes& other)
    {
        if (this != &other) {
            numNotes = other.numNotes;
            timestamps = std::make_unique<unsigned long[]>(other.numNotes);
            for (int i = 0; i < numNotes; i++) {
                timestamps[i] = other.timestamps[i];
            }
        }
        return *this;
    }

    // Comparison operator
    bool operator!=(const Notes& other) const
    {
        if (this->numNotes != other.numNotes) {
            return true;
        }
        for (int i = 0; i < this->numNotes; i++) {
            if (timestamps[i] != other.timestamps[i]) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] int getNumNotes() const
    {
        return numNotes;
    }

    [[nodiscard]] unsigned long get(const int index) const
    {
        if (index >= 0 && index < this->numNotes) {
            return timestamps[index];
        }
        return 0;
    }

private:
    int numNotes = 0;
    std::unique_ptr<unsigned long[]> timestamps;
};

// Stateful filter to prevent old notes from reappearing
class NotesFilter
{
public:
    Notes latest(const Notes& notes, const int num)
    {
        const int numNotes = notes.getNumNotes();
        auto newTimestamps = std::make_unique<unsigned long[]>(numNotes);
        auto used = std::make_unique<bool[]>(numNotes);
        for (int i = 0; i < numNotes; i++) {
            newTimestamps[i] = 0;
            used[i] = false;
        }

        for (int n = 0; n < num; n++) {
            unsigned long maxVal = 0;
            int maxIdx = -1;
            for (int i = 0; i < numNotes; i++) {
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
        for (int i = 0; i < numNotes; i++) {
            const unsigned long timestamp = notes.get(i);
            if (!used[i] && timestamp > cutoffThreshold && timestamp > maxUnselectedTimestamp) {
                maxUnselectedTimestamp = timestamp;
            }
        }
        cutoffThreshold = maxUnselectedTimestamp;

        return Notes(numNotes, newTimestamps.get());
    }

private:
    unsigned long cutoffThreshold = 0;
};

#endif // !defined(APP_NOTES_H)
