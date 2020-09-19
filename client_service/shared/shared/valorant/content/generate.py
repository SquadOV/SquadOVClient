import argparse
import json
import os

def generateHeader(version, inputFname, outputDir):
    with open(inputFname, 'r') as f:
        data = json.load(f)

    agentFname = os.path.join(outputDir, 'agents.h.in')
    mapFname = os.path.join(outputDir, 'maps.h.in')
    equipFname = os.path.join(outputDir, 'equips.h.in')
    gameModeFname = os.path.join(outputDir, 'gameModes.h.in')

    # VALORANT_AGENT(NAME, ID)
    with open(agentFname, 'w') as f:
        for agent in data['Characters']:
            f.write('VALORANT_AGENT({0}, {1})\n'.format(agent['Name'], agent['ID']))

    # VALORANT_MAP(NAME, ID, CODENAME, ASSET)
    with open(mapFname, 'w') as f:       
        for vmap in data['Maps']:
            f.write('VALORANT_MAP({0}, {1}, {2}, {3})\n'.format(vmap['Name'], vmap['ID'], vmap['AssetName'], vmap['AssetPath']))

    # VALORANT_EQUIP(NAME, ID)
    with open(equipFname, 'w') as f:
        for equip in data['Equips']:
            f.write('VALORANT_EQUIP({0}, {1})\n'.format(equip['Name'], equip['ID']))

    # VALORANT_GAME_MODE(NAME, ID, ASSET)
    with open(gameModeFname, 'w') as f:
        for mode in data['GameModes']:
            f.write('VALORANT_GAME_MODE({0}, {1}, {2})\n'.format(mode['Name'], mode['ID'], mode['AssetPath']))

parser = argparse.ArgumentParser()
parser.add_argument('--version', required=True)
parser.add_argument('--input', required=True)
parser.add_argument('--output', required=True)
args = parser.parse_args()
generateHeader(args.version, args.input, args.output)