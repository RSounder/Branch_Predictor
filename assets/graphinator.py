import matplotlib.pyplot as plt
import seaborn as sns
import subprocess
import csv
import re

#write to csv file. args: filename with extension .csv, fieldnames list with header vals, rows with list of dicts paired with fieldnames
def write_csv(filename, fieldnames, rows):
    with open(filename, 'w', encoding='UTF8', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

#given command string as arg, function find_misPred_rate executes cpp file, extracts info using re.search and returns misPred_rate
def find_misPred_rate(inputList):
    console_output = subprocess.run(inputList, stdout=subprocess.PIPE).stdout.decode('utf-8')

    extract_misrate_str1 = "misprediction rate:"
    extract_misrate_str2 = "%"

    if re.search(extract_misrate_str1, console_output):
        idx1 = console_output.index(extract_misrate_str1)
        idx2 = console_output.index(extract_misrate_str2)

        res = console_output[idx1 + len(extract_misrate_str1) + 1: idx2]
    else:
        print("String '{0}' is not present in output" .format(extract_misrate_str1))
        res = "0.0"

    return float(res)

#used to plot single line plots for bimodal bp
def single_line_plot(x, y, filename):
    sns.set_theme(style="darkgrid")
    plot = sns.lineplot(x=x, y=y, marker='o').set(
        title=filename.replace("_trace_",",").replace(".png",""),
        xlabel='m', 
        ylabel='branch misprediction rate'
    )
    plt.savefig(filename)
    plt.clf()

#######################################################################################

benchmark_list = ['gcc_trace.txt', 'jpeg_trace.txt', 'perl_trace.txt']
fieldnames = ['m', 'misprediction_rate']

#bimodal bp : (7 ≤ m ≤ 20) for all benchmarks and get misPred_rate for all
for benchmark in benchmark_list:
    rows = []
    filename = benchmark.replace(".txt","") + '_bimodal.csv'
    print(("({0}/4) working for "+filename).format(str(benchmark_list.index(benchmark)+1)))

    for m_val in range(7,21):
        inputStr = ["./sim", "bimodal", str(m_val), benchmark]
        rows.append({'m': m_val, 'misprediction_rate': find_misPred_rate(inputStr)})
        print("\r\t> Progress\t:{:.2%}".format((range(7,21).index(m_val) + 1)/14), end='')

    print()
    write_csv(filename, fieldnames, rows)
    single_line_plot([f['m'] for f in rows],[f['misprediction_rate'] for f in rows], filename.replace(".csv",".png"))

rows = [] #clearing rows list

print("(4/4) working for "+filename)

filename = 'multiline_gshare.csv'
fieldnames = ['m', 'n', 'misprediction_rate']

plt.figure(figsize=(14,14))
plt.title("gcc, gshare")
plt.xlabel("n")
plt.ylabel("branch misprediction rate")

#7 ≤ m ≤ 20 curve has m+1 datapoints: 0 ≤ n ≤ m. find misPred_list for all m,n and group wrt m
for m_val in range(7, 21):
    misPred_list = []
    for n_val in range(0, m_val + 1):
        inputStr = ["./sim", "gshare", str(m_val), str(n_val), benchmark_list[0]]
        temp = find_misPred_rate(inputStr)
        rows.append({'m': m_val,'n': n_val, 'misprediction_rate': temp})
        misPred_list.append(temp)

    plt.plot([*range(0, m_val+1, 1)], misPred_list, marker = 'o', label = "m = " + str(m_val))
    print("\r\t> Progress\t:{:.2%}".format((range(7,21).index(m_val) + 1)/14), end='')

plt.legend()
plt.savefig("multiline_gshare.png")
plt.clf()

write_csv(filename, fieldnames, rows)
print("\nAll Done")
