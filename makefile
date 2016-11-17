CXXFLAGS  = -Ofast -Wall -std=c++14 -march=native -I/usr/local/include

TARGET    = hexagonal-walk
SRCS      = $(shell ls *.cpp)
OBJS      = $(SRCS:%.cpp=%.o)
DEPS      = $(SRCS:%.cpp=%.d)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

-include $(DEPS)

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -MMD -MP

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)
