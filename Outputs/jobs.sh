#!/bin/bash
 
#================= OPTIONS (s'applique à chaque job du tableau) =========================
#SBATCH --array=0-19             # création d'un tableau de 20 jobs indicés de 0 à 19
#SBATCH --partition=court        # choix de la partition
#SBATCH --ntasks=1               # chaque job possède une seule task
#SBATCH --cpus-per-task=4        # une task nécessite 4 CPU
#SBATCH --mem-per-cpu=16384      # 16 Go de RAM par CPU
#SBATCH --output=_%a      # modifie le nom du fichier de sortie par défaut
 
#========================== TASKS ================================

tab1=(2x/random/German_17_26/30_demands 2x/random/German_17_26/10_demands 2x/random/German_17_26/50_demands 2x/random/German_17_26/40_demands 2x/random/German_17_26/20_demands 2x/random/Nsfnet_14_21/30_demands 2x/random/Nsfnet_14_21/10_demands 2x/random/Nsfnet_14_21/50_demands 2x/random/Nsfnet_14_21/40_demands 2x/random/Nsfnet_14_21/20_demands 2x/random/6nodes9links_6_9/30_demands 2x/random/6nodes9links_6_9/10_demands 2x/random/6nodes9links_6_9/50_demands 2x/random/6nodes9links_6_9/40_demands 2x/random/6nodes9links_6_9/20_demands 2x/random/European_18_40/30_demands 2x/random/European_18_40/10_demands 2x/random/European_18_40/50_demands 2x/random/European_18_40/40_demands 2x/random/European_18_40/20_demands )
echo parametersSet/${tab1[$SLURM_ARRAY_TASK_ID]}
./RSASolver -I ${tab1[$SLURM_ARRAY_TASK_ID]} -O executionOutputs/${tab1[$SLURM_ARRAY_TASK_ID]}/out.txt