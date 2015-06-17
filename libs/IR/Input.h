class Input : public boost::enable_shared_from_this<Input> {
private:
public:
    Input(Block *b);
};

typedef boost::shared_ptr<Input>    InputPtr;
