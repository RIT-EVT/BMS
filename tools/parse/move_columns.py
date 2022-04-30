import pandas as pd

original = pd.read_csv('./dev1_parameters.gg.csv', sep='\t', on_bad_lines='warn')
new = pd.read_csv('./Testing.gg.csv', sep='\t', on_bad_lines='warn')

original = original.drop(columns=[' Default Value'])

print(original.columns)

print(len(original['Class name']))
print(len(new['Class name']))


original[' Default Value'] = new[' Parameter Value']

original.to_csv('output.gg.csv')
