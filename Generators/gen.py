import os
import shutil
import csv

class TestUnit:
    def __init__(self,linkStrategy,transponderStrategy,topology,demandCode):
        self.topology = topology
        self.linkStrategy = linkStrategy
        self.transponderStrategy = transponderStrategy
        self.demandCode = demandCode

def testUnitVerifier(testSet):
    for test in testSet:
        print(test.linkStrategy + " "+ test.transponderStrategy +" "+test.topology+" "+test.demandCode)
        

testSet = []

linkStrategies = [f.name for f in os.scandir("../PedroInstances") if f.is_dir()]
for linkStrategy in linkStrategies:
    transponderStrategies = [f.name for f in os.scandir("../PedroInstances/" + linkStrategy) if f.is_dir()]
    for transponderStrategy in transponderStrategies:
        topologies = [f.name for f in os.scandir("../PedroInstances/" + linkStrategy + "/" + transponderStrategy) if f.is_dir()]
        for topology in topologies:
            demands = [f.name for f in os.scandir("../PedroInstances/" + linkStrategy + "/" + transponderStrategy+ "/" + topology + "/Demands") if f.is_dir()]
            for demand in demands:
                test = TestUnit(linkStrategy,transponderStrategy,topology,demand)
                testSet.append(test)

testUnitVerifier(testSet)


for test in testSet:
    auxOutputFolder = [f.name for f in os.scandir("../Outputs") if f.is_dir()]
    if str(test.linkStrategy) not in auxOutputFolder:
        os.mkdir("../Outputs/" + str(test.linkStrategy))

    auxOutputFolder = [f.name for f in os.scandir("../Outputs"+ "/" + str(test.linkStrategy)) if f.is_dir()]    
    if test.transponderStrategy not in auxOutputFolder:
        os.mkdir("../Outputs/" + str(test.linkStrategy)+"/" + test.transponderStrategy)

    auxOutputFolder = [f.name for f in os.scandir("../Outputs"+ "/" + str(test.linkStrategy)+ "/" + test.transponderStrategy) if f.is_dir()] 
    if test.topology not in auxOutputFolder:
        os.mkdir("../Outputs/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology)
    
    auxOutputFolder = [f.name for f in os.scandir("../Outputs"+ "/" + str(test.linkStrategy)+ "/" + test.transponderStrategy+ "/" + test.topology) if f.is_dir()] 
    if test.demandCode not in auxOutputFolder:
        os.mkdir("../Outputs/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/" + test.demandCode)

    source_file = "../PedroInstances/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/Demands/" + test.demandCode + "/demands_1.csv"
    destination_file = "../Outputs/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/" + test.demandCode + "/Demand.csv"
    shutil.copy(source_file, destination_file)

    source_file = "../PedroInstances/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/Links/" + test.demandCode + "/Link.csv"
    destination_file = "../Outputs/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/" + test.demandCode + "/Link.csv"
    shutil.copy(source_file, destination_file)

    source_file = "../PedroInstances/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/Nodes/" + test.demandCode + "/Nodes.csv"
    destination_file = "../Outputs/" + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/" + test.demandCode + "/Node.csv"
    shutil.copy(source_file, destination_file)


auxOutputFolder = [f.name for f in os.scandir("../Outputs") if f.is_dir()]
if "executionOutputs" not in auxOutputFolder:
    os.mkdir("../Outputs/executionOutputs")
else:
    shutil.rmtree("../Outputs/executionOutputs")
    os.mkdir("../Outputs/executionOutputs")


jobsRows = []
with open("../Inputs/jobsBase.sh", newline='') as jobsFile: 
    aux = csv.reader(jobsFile, delimiter=';', quotechar='|')
    for row in aux:
        jobsRows.append(row)
    jobsFile.close() 
jobsName = "../Outputs/jobs.sh"
with open(jobsName, "w") as f:
    counter = 1
    for row in jobsRows:
        if counter == 4:
            line = "#SBATCH --array=0-"+str(len(testSet)-1)+"             # création d'un tableau de "+str(len(testSet))+ " jobs indicés de 0 à "+str(len(testSet)-1)
            f.write(str(line)+"\n")
        else:
            if counter == 9:
                line = "#SBATCH --output=_%a      # modifie le nom du fichier de sortie par défaut"
                f.write(str(line)+"\n")
            else:
                line = row[0]
                f.write(str(line)+"\n")
        counter = counter + 1

    f.write("\n")
    stringLine1 = "tab1=("
    for test in testSet:
        stringLine1 = stringLine1 + str(test.linkStrategy)+"/" + test.transponderStrategy +"/" + test.topology + "/" + test.demandCode + " "
    stringLine1 = stringLine1 + ")\n"
    f.write(stringLine1)
    echoLine = "echo parametersSet/${tab1[$SLURM_ARRAY_TASK_ID]}\n"
    f.write(echoLine)
    lastLine = "./RSASolver -I ${tab1[$SLURM_ARRAY_TASK_ID]} -O executionOutputs/${tab1[$SLURM_ARRAY_TASK_ID]}/out.txt" 
    f.write(lastLine)
    f.close() 
print("Jobs script created")

