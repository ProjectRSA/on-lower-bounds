#!/bin/bash
 
#================= OPTIONS (s'applique à chaque job du tableau) =========================
#SBATCH --array=0-8            # création d'un tableau de 9 jobs indicés de 0 à 8
#SBATCH --partition=court        # choix de la partition
#SBATCH --ntasks=1               # chaque job possède une seule task
#SBATCH --cpus-per-task=4        # une task nécessite 4 CPU
#SBATCH --mem-per-cpu=16384      # 16 Go de RAM par CPU
#SBATCH --output=batch_1_%a      # modifie le nom du fichier de sortie par défaut
 
#========================== TASKS ================================

tab1=(Nsfnet_14_21/30 Nsfnet_14_21/40 Nsfnet_14_21/50 European_18_40/30 European_18_40/40 European_18_40/50 German_17_26/30 German_17_26/40 German_17_26/50 )
echo UserInstances/QoT_Instances/PAPER_kShortest5/${tab1[$SLURM_ARRAY_TASK_ID]}
./exe1 UserInstances/QoT_Instances/PAPER_kShortest5/${tab1[$SLURM_ARRAY_TASK_ID]} UserInstances/QoT_Instances/PAPER_kShortest5/${tab1[$SLURM_ARRAY_TASK_ID]}.txt 5