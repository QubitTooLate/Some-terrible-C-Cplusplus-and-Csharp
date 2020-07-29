static class Program
{
    static void Main(string[] args)
    {
        var piramid_width = 69;
        var piramid_bricks = new string('#', piramid_width).AsMemory();
        var air = new string(' ', piramid_width / 2).AsMemory();

        for (int i = piramid_width / 2; i >= 0; --i)
        {
            Console.Write(air[..i]);
            Console.WriteLine(piramid_bricks[i..(piramid_width - i)]);
        }
    }
}
