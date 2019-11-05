from optparse import OptionParser

def get_opts():
    parser = OptionParser()

    parser.add_option("-c", "--bench-path", help="Binary of the program to be simulated")
    parser.add_option("--input-path", help="Fault input file")
    parser.add_option("--output", help="Output", default="")
    parser.add_option("--cache-level", help="Cache Level", default="1")

    # Cache Options
    parser.add_option("--l1d-size", type="string", default="64kB")
    parser.add_option("--l1i-size", type="string", default="32kB")
    parser.add_option("--l2-size", type="string", default="2MB")
    parser.add_option("--l3-size", type="string", default="16MB")
    parser.add_option("--l1d-assoc", type="int", default=2)
    parser.add_option("--l1i-assoc", type="int", default=2)
    parser.add_option("--l2-assoc", type="int", default=8)
    parser.add_option("--l3-assoc", type="int", default=16)

    # Options for dct application : example run: ./dct <inputFile> <outputFile>
    parser.add_option("--dct-input", help="Input file for dct application", default="")

    # Options for blackscholes application : example run: ./blackscholes <inputFile> <outputFile>
    parser.add_option("--blackscholes-input", help="Input file for blackscholes application", default="")

    # Options for jacobi application : example run: ./jacobi 1000 0.00000001 1 100 $outputFileName
    parser.add_option("--jacobi-n", help="Size of matrix", default="")
    parser.add_option("--jacobi-itol", help="Itol", default="")
    parser.add_option("--jacobi-dominant", help="Is diagonally dominant?", default="")
    parser.add_option("--jacobi-maxiters", help="Maximum iteration", default="")

    # Options for kmeans application : example run: seq_main -o -b -n 4 -i Image_data/color17695.bin
    parser.add_option("--kmeans-o", action="store_true", help="output timing results")
    parser.add_option("--kmeans-b", action="store_true", help="input file is in binary format")
    parser.add_option("--kmeans-n", help="number of clusters")
    parser.add_option("--kmeans-i", help="file containing data to be clustered")

    # Options for monte carlo application : example run: ./monte_carlo  5 5 50 5 out.bin
    parser.add_option("--monte-x", help="Size of X", default="")
    parser.add_option("--monte-y", help="Size of Y", default="")
    parser.add_option("--monte-walks", help="Is diagonally dominant?", default="")
    parser.add_option("--monte-tasks", help="Maximum iteration", default="")

    # Options for sobel application : example run: ./sobel 'input file' 'output file' 
    parser.add_option("--sobel-input", help="Input file", default="")

    return parser.parse_args()