<Query Kind="Program" />

void Main()
{
    IsValid("{}").Dump("Is Valid ?");
}

bool IsValid(string stream)
{
    var (pos, _) = Parse(stream, 0);
    return pos == stream.Length;
}

enum ParserType 
{
    Object,
    Array,
    String,
    Int
}

static Dictionary<ParserType, Func<string, int, int>> Parsers = new()
{
    [ParserType.Object] = ParseObject,
    [ParserType.Array] = ParseArray,
    [ParserType.String] = ParseString,
    [ParserType.Int] = ParseInt
};

static (int Pos, ParserType? ParserType) Parse(string stream, int pos) 
{
    pos = SkipWhitespace(stream, pos);
    ParserType? parserType = null;
    foreach (var parser in Parsers)
    {
        var nextPos = parser.Value(stream, pos);
        if (nextPos > pos)
        {
            pos = SkipWhitespace(stream, nextPos);
            parserType = parser.Key;
            break;
        }
    }
    return (pos, parserType);
}

static int ParseObject(string stream, int pos)
{
    int initialPos = pos;
    bool acceptChild = false, done = false;
    for (; pos < stream.Length; ++pos)
    {
        if (done)
            break;

        char c = stream[pos];

        if (c == '{' || c == ',')
        {
            var nextPos = SkipWhitespace(stream, pos + 1);
            if (nextPos > pos) pos = nextPos - 1;
            acceptChild = true;
            continue;
        }

        if (c == '}' && pos > initialPos)
        {
            done = true;
            continue;
        }

        if (acceptChild)
        {
            var afterKeyPos = ParseString(stream, pos);
            if (afterKeyPos == pos) break;
            afterKeyPos = SkipWhitespace(stream, afterKeyPos);
            if (afterKeyPos == stream.Length) break;

            var nextc = stream[afterKeyPos];
            if (nextc != ':') break;
            var childPos = SkipWhitespace(stream, afterKeyPos + 1);
            var child = Parse(stream, childPos);

            if (child.Pos > pos)
            {
                pos = child.Pos - 1;
                acceptChild = false;
                continue;
            }
            else
                break;
        }

        break;
    }
    return done ? pos : initialPos;
}

static int ParseArray(string stream, int pos)
{
    int initialPos = pos;
    bool acceptChild = false, done = false;
    for (; pos < stream.Length; ++pos)
    {
        if (done)
            break;

        char c = stream[pos];
        
        if (c == '[' || c == ',') 
        {
            acceptChild = true;
            continue;
        }
        
        if (c == ']' && pos > initialPos) 
        {
            done = true;
            continue;
        }

        if (acceptChild)
        {
            var child = Parse(stream, pos);
            if (child.Pos > pos)
            {
                pos = child.Pos - 1;
                acceptChild = false;
                continue;
            }
            else
                break;
        }

        break;
    }
    return done ? pos : initialPos;
}

static int ParseString(string stream, int pos)
{
    int initialPos = pos, numQuotes = 0;
    for (; pos < stream.Length; ++pos)
    {
        if (numQuotes == 2)
            break;

        char c = stream[pos];
        
        if (c == '"')
        {
            ++numQuotes;
            continue;
        }
        else if (numQuotes == 0)
            break;
    }
    return numQuotes == 2 ? pos : initialPos;
}

static int ParseInt(string stream, int pos)
{
    for (; pos < stream.Length; ++pos)
    {
        char c = stream[pos];
        
        if (c < '0' || c > '9')
            break;
    }
    return pos;
}

static int SkipWhitespace(string stream, int pos)
{
    for (; pos < stream.Length; ++pos)
    {
        char c = stream[pos];
        
        if (c != ' ' && c != '\r' && c != '\n' && c != '\t')
            break;
    }
    return pos;
}

