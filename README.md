# cs4223-cache-coherence


# Assumption

201014: 

no of compute cycles not include idle time

invalidate takes 1 cache access = 1 cycle

write -> dirty

use Illinois MESI 

201022:

cache states: busy - someone is accessing, idle_wait - waiting for busOp

tie-break for busOp = core/cache ID -> core/cache has ID

