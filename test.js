// f4ce36d88ab8b6fd

// A082

// 00112233445566778899AABBCCDDEEFF528F

// ZB10SG0D83101823480040000000000000000000F4CE36D88AB8B6FDDLK00112233445566778899AABBCCDDEEFF528F

// Z:F4CE36D88AB8B6FD$I:00112233445566778899AABBCCDDEEFF528F%M:FFFE$D:AABBCCDD

ZB10SG0D83101823480040000000000000000000F4CE36B6202D3DDBDLK00112233445566778899AABBCCDDEEFF528F

function addInstallCode(installCode) {
    const aqaraMatch = installCode.match(/^G\$M:.+\$A:(.+)\$I:(.+)$/);
    let ieeeAddr, key;
    if (aqaraMatch) {
        ieeeAddr = aqaraMatch[1];
        key = aqaraMatch[2];
    } else {
        const keyStart = installCode.length - (installCode.length === 95 ? 36 : 32);
        ieeeAddr = installCode.substring(keyStart - 19, keyStart - 3);
        key = installCode.substring(keyStart, installCode.length);
    }

    ieeeAddr = `0x${ieeeAddr}`;
    key = Buffer.from(key.match(/.{1,2}/g).map(d => parseInt(d, 16)));
}

addInstallCode("ZB10SG0D831018234800400000000000000000009035EAFFFE424793DLKAE3B287281CF11F550733A0CFC38AA31E802");