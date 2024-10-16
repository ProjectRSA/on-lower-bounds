#!/bin/bash
 
#================= OPTIONS (s'applique à chaque job du tableau) =========================
#SBATCH --array=0-39             # création d'un tableau de 40 jobs indicés de 0 à 39
#SBATCH --partition=court        # choix de la partition
#SBATCH --ntasks=1               # chaque job possède une seule task
#SBATCH --cpus-per-task=4        # une task nécessite 4 CPU
#SBATCH --mem-per-cpu=16384      # 16 Go de RAM par CPU
#SBATCH --output=_%a      # modifie le nom du fichier de sortie par défaut
 
#========================== TASKS ================================

tab1=(2x/random/German_17_26/80_demands 2x/random/German_17_26/100_demands 2x/random/German_17_26/30_demands 2x/random/German_17_26/90_demands 2x/random/German_17_26/60_demands 2x/random/German_17_26/10_demands 2x/random/German_17_26/50_demands 2x/random/German_17_26/70_demands 2x/random/German_17_26/40_demands 2x/random/German_17_26/20_demands 2x/random/Nsfnet_14_21/80_demands 2x/random/Nsfnet_14_21/100_demands 2x/random/Nsfnet_14_21/30_demands 2x/random/Nsfnet_14_21/90_demands 2x/random/Nsfnet_14_21/60_demands 2x/random/Nsfnet_14_21/10_demands 2x/random/Nsfnet_14_21/50_demands 2x/random/Nsfnet_14_21/70_demands 2x/random/Nsfnet_14_21/40_demands 2x/random/Nsfnet_14_21/20_demands 2x/random/6nodes9links_6_9/80_demands 2x/random/6nodes9links_6_9/100_demands 2x/random/6nodes9links_6_9/30_demands 2x/random/6nodes9links_6_9/90_demands 2x/random/6nodes9links_6_9/60_demands 2x/random/6nodes9links_6_9/10_demands 2x/random/6nodes9links_6_9/50_demands 2x/random/6nodes9links_6_9/70_demands 2x/random/6nodes9links_6_9/40_demands 2x/random/6nodes9links_6_9/20_demands 2x/random/European_18_40/80_demands 2x/random/European_18_40/100_demands 2x/random/European_18_40/30_demands 2x/random/European_18_40/90_demands 2x/random/European_18_40/60_demands 2x/random/European_18_40/10_demands 2x/random/European_18_40/50_demands 2x/random/European_18_40/70_demands 2x/random/European_18_40/40_demands 2x/random/European_18_40/20_demands )
echo parametersSet/${tab1[$SLURM_ARRAY_TASK_ID]}
./RSASolver -I ${tab1[$SLURM_ARRAY_TASK_ID]} -O executionOutputs/${tab1[$SLURM_ARRAY_TASK_ID]}/out.txt -S executionOutputs/${tab1[$SLURM_ARRAY_TASK_ID]}/sum.csv