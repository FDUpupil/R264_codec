#ifndef __MAP_2D_H__
#define __MAP_2D_H__

template <typename T, typename IndexType = uint16_t>
class Map2D {
public:
    Map2D(IndexType _width, IndexType _height)
        : width(_width), height(_height)
    {
        array = new T[width * height];
    }

    ~Map2D()
    {
        delete[] array;
    }

    inline T * operator[](IndexType y)
    {
        return &array[y * width];
    }

    inline const T * operator[](IndexType y) const
    {
        return &array[y * width];
    }

    IndexType getWidth() const { return width; }
    IndexType getHeight() const { return height; }

private:
    IndexType width;
    IndexType height;
    T *array;
};

#endif
