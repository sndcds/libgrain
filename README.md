# libgrain

```bash
cmake --build build
sudo cmake --install build --prefix /usr/local
```

## Checklist for Classes

- Constructor (const char* csv, char delimiter)
- className()
- << operator with *o and &o
- writeToFile(File& f)
- readFromFile(File& f)
- setBySCV(const char* csv, char delimiter)