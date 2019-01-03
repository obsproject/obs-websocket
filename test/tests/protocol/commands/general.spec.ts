import obs from "../../../obs";
import { expect } from 'chai';

describe('General', () => {
    it('GetVersion succeeds', async () => {
        const response = await obs.send('GetVersion');
        expect(response['obs-websocket-version']).to.eql('4.5.0');
        expect(response['obs-studio-version']).to.not.be.empty;
        expect(response['available-requests']).to.not.be.empty;
    });

    it('GetAuthRequired succeeds', async () => {
        const response = await obs.send('GetAuthRequired');
        expect(response.authRequired).to.eql(true);
        expect(response.challenge).to.not.be.empty;
        expect(response.salt).to.not.be.empty;
    });
});
